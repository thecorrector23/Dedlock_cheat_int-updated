#pragma once
#include <windows.h>
#include <chrono>
#include "ImGui/imgui.h"

#ifndef MOVEMENT_H
#define MOVEMENT_H

namespace Movement {
    extern bool script_enabled;
    extern int delay_before_space;
    extern int delay_before_ctrl;
    extern int initial_shift_delay;

    void process_movement_logic();
    void render_movement_settings();
}

#endif // MOVEMENT_H