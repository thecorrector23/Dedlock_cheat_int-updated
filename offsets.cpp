#include "offsets.h"
#include "memory.h" 
#include <vector>
#include <string>
#include <iostream>

namespace offsets
{
    //Replace the dumped offsets here (random offsets btw)
    std::ptrdiff_t dwEntityList = 432424;
    std::ptrdiff_t dwLocalPlayer = 6546468;
    std::ptrdiff_t CCitadelCameraManager = 76575756;
    std::ptrdiff_t dwViewMatrix = 757575756;

    void initializeOffsets() //Not necessary to print
    {
        //std::cout << "dwEntityList: " << std::hex << dwEntityList << std::endl;
        //std::cout << "dwLocalPlayer: " << std::hex << dwLocalPlayer << std::endl;
        //std::cout << "CCitadelCameraManager: " << std::hex << CCitadelCameraManager << std::endl;
        //std::cout << "dwViewMatrix: " << std::hex << dwViewMatrix << std::endl;
    }
}