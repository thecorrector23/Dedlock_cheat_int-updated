#pragma once
#include <string>
#include <Windows.h>
#include <chrono>
#include "ImGui/imgui.h"

struct Config {
    bool esp_enabled = false;
    bool esp_show_head = false;
    bool esp_show_circle = false;
    float esp_distance_radius = 1000.0f;
    bool esp_box_enabled = false;
    bool esp_line_enabled = false;
    bool esp_health_enabled = false;
    bool show_health = false;
    bool show_max_health = false;
    bool esp_show_address = false;
    bool esp_show_name = false;
    float boxThickness = 1.0f;
    float lineThickness = 1.0f;
    ImVec4 boxColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    ImVec4 lineColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

    bool aimbot_enabled = false;
    bool aimbot_active = false;
    float aimbot_fov = 90.0f;
    float aimbot_smoothness = 1.0f;
    int aimbot_activation_key = VK_LBUTTON;
    float aimbot_max_distance = 1000.0f;
    bool aimbot_soulsAim = false;

    bool hook_active = false;
    int hook_speed = 0;
    float aim_smoothness = 1.0f;
    bool enable_delay = false;
    int hookActivationKey = 0;

    bool script_enabled = true;
    int delay_before_space = 300;
    int delay_before_ctrl = 1000;
    int initial_shift_delay = 250;

    bool sniper_aim_active = false;
    float sniper_search_radius = 3000.0f;
    float sniper_hp_threshold = 1.0f;
};

extern Config config;

void LoadConfig(const std::string& filename);
void SaveConfig(const std::string& filename);
void RenderConfigMenu();