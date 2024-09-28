#include "offsets.h"
#include "memory.h" 
#include <vector>
#include <string>
#include <iostream>

namespace offsets
{
    std::ptrdiff_t dwEntityList = 0x1f4fb98;
    std::ptrdiff_t dwLocalPlayer = 0x20fd718;
    std::ptrdiff_t CCitadelCameraManager = 0x1f71b00;
    std::ptrdiff_t dwViewMatrix = 0x210f420;

    void initializeOffsets()
    {
        std::cout << "dwEntityList: " << std::hex << dwEntityList << std::endl;
        std::cout << "dwLocalPlayer: " << std::hex << dwLocalPlayer << std::endl;
        std::cout << "CCitadelCameraManager: " << std::hex << CCitadelCameraManager << std::endl;
        std::cout << "dwViewMatrix: " << std::hex << dwViewMatrix << std::endl;
    }
}
