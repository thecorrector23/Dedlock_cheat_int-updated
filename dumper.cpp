#include "dumper.h"
#include "offsets.h"
#include <TlHelp32.h>
#include <Psapi.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ShlObj.h>
#include "ImGui/imgui.h"

namespace dumper {
    uintptr_t dumpedEntityList = 0;
    uintptr_t dumpedLocalPlayer = 0;
    uintptr_t dumpedCCameraManager = 0;
    uintptr_t dumpedViewMatrix = 0;

    Signature::Signature(const std::string& pattern, int offset, int extra) : pattern(pattern), offset(offset), extra(extra) {}

    std::vector<uint8_t> Signature::parse_pattern() const {
        std::vector<uint8_t> bytes;
        std::istringstream patternStream(pattern);
        std::string byteStr;

        while (patternStream >> byteStr) {
            if (byteStr == "?" || byteStr == "??") {
                bytes.push_back(0);
            }
            else {
                bytes.push_back(static_cast<uint8_t>(strtol(byteStr.c_str(), nullptr, 16)));
            }
        }
        return bytes;
    }

    void Signature::find(const std::vector<uint8_t>& memory, HANDLE processHandle, uintptr_t moduleBase, uintptr_t& offsetVar) const {
        std::vector<uint8_t> pattern = parse_pattern();
        for (size_t i = 0; i < memory.size() - pattern.size(); ++i) {
            bool patternMatch = true;
            for (size_t j = 0; j < pattern.size(); ++j) {
                if (pattern[j] != 0 && memory[i + j] != pattern[j]) {
                    patternMatch = false;
                    break;
                }
            }
            if (patternMatch) {
                uintptr_t patternAddress = moduleBase + i;
                int32_t of;
                if (ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(patternAddress + offset), &of, sizeof(of), nullptr)) {
                    offsetVar = patternAddress + of + extra;
                    break;
                }
            }
        }
    }

    std::string wstringToString(const std::wstring& wstr) {
        return std::string(wstr.begin(), wstr.end());
    }

    HANDLE getProcessHandle(const std::string& processName) {
        PROCESSENTRY32 processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32);
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (Process32First(snapshot, &processEntry)) {
            do {
                std::string exeFileName = wstringToString(processEntry.szExeFile);
                if (processName == exeFileName) {
                    CloseHandle(snapshot);
                    return OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, processEntry.th32ProcessID);
                }
            } while (Process32Next(snapshot, &processEntry));
        }
        CloseHandle(snapshot);
        return nullptr;
    }

    MODULEINFO getModuleInfo(HANDLE processHandle, const std::string& moduleName) {
        HMODULE hMods[1024];
        DWORD cbNeeded;
        MODULEINFO modInfo = { 0 };

        if (EnumProcessModules(processHandle, hMods, sizeof(hMods), &cbNeeded)) {
            for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
                char szModName[MAX_PATH];
                if (GetModuleBaseNameA(processHandle, hMods[i], szModName, sizeof(szModName) / sizeof(char))) {
                    if (moduleName == szModName) {
                        GetModuleInformation(processHandle, hMods[i], &modInfo, sizeof(modInfo));
                        break;
                    }
                }
            }
        }
        return modInfo;
    }

    std::vector<uint8_t> readMemoryBytes(HANDLE processHandle, uintptr_t address, size_t size) {
        std::vector<uint8_t> buffer(size);
        ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(address), buffer.data(), size, nullptr);
        return buffer;
    }

    void DumpOffsets() {
        Signature localPlayerSig("48 8B 0D ? ? ? ? 48 85 C9 74 65 83 FF FF", 3, 7);
        Signature viewMatrixSig("48 8D ? ? ? ? ? 48 C1 E0 06 48 03 C1 C3", 3, 7);
        Signature entityListSig("48 8B 0D ? ? ? ? 8B C5 48 C1 E8", 3, 7);
        Signature CCameraManagerSig("48 8D 3D ? ? ? ? 8B D9", 3, 7);

        std::string processName = "project8.exe";
        HANDLE processHandle = getProcessHandle(processName);
        if (!processHandle) {
            std::cerr << "Game process not found" << std::endl;
            return;
        }

        MODULEINFO moduleInfo = getModuleInfo(processHandle, "client.dll");
        if (!moduleInfo.lpBaseOfDll) {
            std::cerr << "client.dll not found" << std::endl;
            CloseHandle(processHandle);
            return;
        }

        uintptr_t moduleBase = reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll);

        std::vector<uint8_t> memory = readMemoryBytes(processHandle, moduleBase, moduleInfo.SizeOfImage);

        localPlayerSig.find(memory, processHandle, moduleBase, dumpedLocalPlayer);
        viewMatrixSig.find(memory, processHandle, moduleBase, dumpedViewMatrix);
        entityListSig.find(memory, processHandle, moduleBase, dumpedEntityList);
        CCameraManagerSig.find(memory, processHandle, moduleBase, dumpedCCameraManager);

        CloseHandle(processHandle);

        dumpedEntityList -= moduleBase;
        dumpedLocalPlayer -= moduleBase;
        dumpedViewMatrix -= moduleBase;
        dumpedCCameraManager -= moduleBase;

        bool offsetsUpdated = false;

        if (offsets::dwEntityList != dumpedEntityList) {
            std::cout << "Updating dwEntityList from 0x" << std::hex << offsets::dwEntityList
                << " to 0x" << dumpedEntityList << std::endl;
            offsets::dwEntityList = dumpedEntityList;
            offsetsUpdated = true;
        }

        if (offsets::dwLocalPlayer != dumpedLocalPlayer) {
            std::cout << "Updating dwLocalPlayer from 0x" << std::hex << offsets::dwLocalPlayer
                << " to 0x" << dumpedLocalPlayer << std::endl;
            offsets::dwLocalPlayer = dumpedLocalPlayer;
            offsetsUpdated = true;
        }

        if (offsets::dwViewMatrix != dumpedViewMatrix) {
            std::cout << "Updating dwViewMatrix from 0x" << std::hex << offsets::dwViewMatrix
                << " to 0x" << dumpedViewMatrix << std::endl;
            offsets::dwViewMatrix = dumpedViewMatrix;
            offsetsUpdated = true;
        }

        if (offsets::CCitadelCameraManager != dumpedCCameraManager) {
            std::cout << "Updating CCitadelCameraManager from 0x" << std::hex << offsets::CCitadelCameraManager
                << " to 0x" << dumpedCCameraManager << std::endl;
            offsets::CCitadelCameraManager = dumpedCCameraManager;
            offsetsUpdated = true;
        }

        if (offsetsUpdated) {
            std::cout << "Offsets updated." << std::endl;
        }
        else {
            std::cout << "Offsets are already updated." << std::endl;
        }
    }
}
