#ifndef DUMPER_H
#define DUMPER_H

#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <vector>
#include <string>
#include <Psapi.h>
#include <sstream>
#include <fstream>

namespace dumper {
    class Signature {
    public:
        Signature(const std::string& pattern, int offset, int extra);
        std::vector<uint8_t> parse_pattern() const;
        void find(const std::vector<uint8_t>& memory, HANDLE processHandle, uintptr_t moduleBase, uintptr_t& offsetVar) const;
    private:
        std::string pattern;
        int offset;
        int extra;
    };

    HANDLE getProcessHandle(const std::string& processName);
    MODULEINFO getModuleInfo(HANDLE processHandle, const std::string& moduleName);
    std::vector<uint8_t> readMemoryBytes(HANDLE processHandle, uintptr_t address, size_t size);
    std::string wstringToString(const std::wstring& wstr);

    extern uintptr_t dumpedEntityList;
    extern uintptr_t dumpedLocalPlayer;
    extern uintptr_t dumpedViewMatrix;
    extern uintptr_t dumpedCCameraManager;

    void DumpOffsets();
    void RenderDumpMenu();
}

#endif