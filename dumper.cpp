#include "dumper.h"
#include <TlHelp32.h>
#include <Psapi.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ShlObj.h>
#include "ImGui/imgui.h"

namespace dumper {
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

    void Signature::find(const std::vector<uint8_t>& memory, HANDLE processHandle, uintptr_t moduleBase, std::ofstream& outFile) const {
        std::vector<uint8_t> pattern = parse_pattern();
        for (size_t i = 0; i < memory.size(); ++i) {
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
                    uintptr_t result = patternAddress + of + extra;
                    outFile << "+ 0x" << std::hex << (result - moduleBase) << std::endl;
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
            //std::cerr << "Game process not found" << std::endl;
            return;
        }

        MODULEINFO moduleInfo = getModuleInfo(processHandle, "client.dll");
        if (!moduleInfo.lpBaseOfDll) {
            //std::cerr << "client.dll not found" << std::endl;
            CloseHandle(processHandle);
            return;
        }

        std::vector<uint8_t> memory = readMemoryBytes(processHandle, reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll), moduleInfo.SizeOfImage);

        char desktopPath[MAX_PATH];
        SHGetSpecialFolderPathA(nullptr, desktopPath, CSIDL_DESKTOP, FALSE);

        std::ofstream outFile(std::string(desktopPath) + "\\DeadlockOffsets.txt");
        if (!outFile.is_open()) {
            //std::cerr << "Failed to create file on desktop" << std::endl;
            CloseHandle(processHandle);
            return;
        }

        outFile << "LocalPlayerController:" << std::endl;
        localPlayerSig.find(memory, processHandle, reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll), outFile);
        outFile << "ViewMatrix:" << std::endl;
        viewMatrixSig.find(memory, processHandle, reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll), outFile);
        outFile << "EntityList:" << std::endl;
        entityListSig.find(memory, processHandle, reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll), outFile);
        outFile << "CCameraManager:" << std::endl;
        CCameraManagerSig.find(memory, processHandle, reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll), outFile);

        outFile.close();
        CloseHandle(processHandle);
    }

    void RenderDumpMenu() {
        ImGui::Separator();
        ImGui::Text("Dump the current Offsets to your Desktop");
        ImGui::Text("To update them, replace the old Offsets with the new ones in the offsets.cpp");
        if (ImGui::Button("Dump Offsets")) {
            DumpOffsets();
            ImGui::Text("Offsets dumped to desktop");
        }
    }
}