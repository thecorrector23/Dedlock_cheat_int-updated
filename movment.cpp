#include "movment.h"
#include <Windows.h>
#include <chrono>
#include "ImGui/imgui.h"
#include "config.h"
#include "movment.h"
#include <chrono>

namespace Movement {
    bool shift_held = false, ctrl_held = false, space_pressed = false, script_enabled = true;
    auto shift_press_time = std::chrono::steady_clock::now();
    int delay_before_space = 300, delay_before_ctrl = 1000, initial_shift_delay = 250;
    int min_delay = 100, max_delay = 2000;

    void process_movement_logic() {
        if (!script_enabled) return;

        if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
            if (!shift_held) {
                shift_held = true;
                space_pressed = false;
                shift_press_time = std::chrono::steady_clock::now();
            }
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - shift_press_time).count();

            if (duration > initial_shift_delay && duration < delay_before_ctrl) {
                if (duration > delay_before_space && !space_pressed) {
                    keybd_event(VK_SPACE, 0, 0, 0);
                    keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0);
                    space_pressed = true;
                }
            }
            else if (duration >= config.delay_before_ctrl && !ctrl_held) {
                keybd_event(VK_CONTROL, 0, 0, 0);
                ctrl_held = true;
            }
        }
        else {
            if (shift_held && ctrl_held) keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
            shift_held = ctrl_held = space_pressed = false;
        }
    }
    void render_movement_settings() {
        ImGui::Checkbox("Script Enabled", &config.script_enabled);
        ImGui::SliderInt("Delay Before Space", &config.delay_before_space, 0, 1000);
        ImGui::SliderInt("Delay Before Ctrl", &config.delay_before_ctrl, 0, 1000);
        ImGui::SliderInt("Initial Shift Delay", &config.initial_shift_delay, 0, 1000);
    }
}