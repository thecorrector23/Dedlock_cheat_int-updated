#include "offsets.h"
#include "memory.h" 
#include <vector>
#include <string>
#include <iostream>

namespace offsets
{
    //Replace the dumped offsets here
    std::ptrdiff_t dwEntityList = 0x1f50c28;
    std::ptrdiff_t dwLocalPlayer = 0x20fe768;
    std::ptrdiff_t CCitadelCameraManager = 0x1f72b60;
    std::ptrdiff_t dwViewMatrix = 0x2110470;

    void initializeOffsets() //Not necessary to print
    {
        //std::cout << "dwEntityList: " << std::hex << dwEntityList << std::endl;
        //std::cout << "dwLocalPlayer: " << std::hex << dwLocalPlayer << std::endl;
        //std::cout << "CCitadelCameraManager: " << std::hex << CCitadelCameraManager << std::endl;
        //std::cout << "dwViewMatrix: " << std::hex << dwViewMatrix << std::endl;
    }
}