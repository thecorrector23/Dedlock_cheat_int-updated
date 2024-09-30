#include "config.h"
#include "aimbot.h"
#include "visuals.h"
#include "beepbob.h"
#include "sniper.h"
#include "movment.h"
#include <fstream>
#include <iostream>
#include "json.hpp"

using json = nlohmann::json;

Config config;

void LoadConfig() {
    std::ifstream inputFile("config.json");
    if (!inputFile.is_open()) {
        //std::cout << "Failed to open config.json for reading.\n";
        return;
    }

    json j;
    inputFile >> j;
    inputFile.close();

    Aimbot::settings.enabled = j["aimbot"]["enabled"].get<bool>();
    Aimbot::settings.fov = j["aimbot"]["fov"].get<float>();
    Aimbot::settings.smoothness = j["aimbot"]["smoothness"].get<float>();
    Aimbot::settings.maxDistance = j["aimbot"]["maxDistance"].get<float>();
    Aimbot::settings.soulsAim = j["aimbot"]["soulsAim"].get<bool>();
    Aimbot::settings.activationKey = j["aimbot"]["activationKey"].get<int>();

    Visuals1::esp_show_head = j["esp"]["show_head"].get<bool>();
    Visuals1::esp_show_circle = j["esp"]["show_circle"].get<bool>();
    Visuals1::esp_distance_radius = j["esp"]["distance_radius"].get<float>();
    Visuals1::esp_box_enabled = j["esp"]["box_enabled"].get<bool>();
    Visuals1::esp_line_enabled = j["esp"]["line_enabled"].get<bool>();
    Visuals1::esp_health_enabled = j["esp"]["health_enabled"].get<bool>();
    Visuals1::show_health = j["esp"]["show_health"].get<bool>();
    Visuals1::show_max_health = j["esp"]["show_max_health"].get<bool>();
    Visuals1::esp_show_address = j["esp"]["show_address"].get<bool>();
    Visuals1::esp_show_name = j["esp"]["show_name"].get<bool>();
    Visuals1::boxThickness = j["esp"]["box_thickness"].get<float>();
    Visuals1::lineThickness = j["esp"]["line_thickness"].get<float>();
    Visuals1::boxColor = ImVec4(
        j["esp"]["box_color"][0].get<float>(),
        j["esp"]["box_color"][1].get<float>(),
        j["esp"]["box_color"][2].get<float>(),
        j["esp"]["box_color"][3].get<float>()
    );
    Visuals1::lineColor = ImVec4(
        j["esp"]["line_color"][0].get<float>(),
        j["esp"]["line_color"][1].get<float>(),
        j["esp"]["line_color"][2].get<float>(),
        j["esp"]["line_color"][3].get<float>()
    );

    BebopHook::hook_active = j.value("hook_active", BebopHook::hook_active);
    BebopHook::hook_speed = j.value("hook_speed", BebopHook::hook_speed);
    BebopHook::aim_smoothness = j.value("aim_smoothness", BebopHook::aim_smoothness);
    BebopHook::enable_delay = j.value("enable_delay", BebopHook::enable_delay);
    BebopHook::hookActivationKey = j.value("hook_activation_key", BebopHook::hookActivationKey);

    config.script_enabled = j["movement"]["script_enabled"].get<bool>();
    config.delay_before_space = j["movement"]["delay_before_space"].get<int>();
    config.delay_before_ctrl = j["movement"]["delay_before_ctrl"].get<int>();
    config.initial_shift_delay = j["movement"]["initial_shift_delay"].get<int>();

    SniperAutoAim::aim_active = j["sniper"]["aim_active"].get<bool>();
    SniperAutoAim::search_radius = j["sniper"]["search_radius"].get<float>();
    SniperAutoAim::hp_threshold = j["sniper"]["hp_threshold"].get<float>();

    //std::cout << "Loaded config settings from config.json.\n";
}

void SaveConfig() {
    json j;

    j["aimbot"]["enabled"] = Aimbot::settings.enabled;
    j["aimbot"]["fov"] = Aimbot::settings.fov;
    j["aimbot"]["smoothness"] = Aimbot::settings.smoothness;
    j["aimbot"]["maxDistance"] = Aimbot::settings.maxDistance;
    j["aimbot"]["soulsAim"] = Aimbot::settings.soulsAim;
    j["aimbot"]["activationKey"] = Aimbot::settings.activationKey;

    j["esp"]["show_head"] = Visuals1::esp_show_head;
    j["esp"]["show_circle"] = Visuals1::esp_show_circle;
    j["esp"]["distance_radius"] = Visuals1::esp_distance_radius;
    j["esp"]["box_enabled"] = Visuals1::esp_box_enabled;
    j["esp"]["line_enabled"] = Visuals1::esp_line_enabled;
    j["esp"]["health_enabled"] = Visuals1::esp_health_enabled;
    j["esp"]["show_health"] = Visuals1::show_health;
    j["esp"]["show_max_health"] = Visuals1::show_max_health;
    j["esp"]["show_address"] = Visuals1::esp_show_address;
    j["esp"]["show_name"] = Visuals1::esp_show_name;
    j["esp"]["box_thickness"] = Visuals1::boxThickness;
    j["esp"]["line_thickness"] = Visuals1::lineThickness;
    j["esp"]["box_color"] = { Visuals1::boxColor.x, Visuals1::boxColor.y, Visuals1::boxColor.z, Visuals1::boxColor.w };
    j["esp"]["line_color"] = { Visuals1::lineColor.x, Visuals1::lineColor.y, Visuals1::lineColor.z, Visuals1::lineColor.w };

    j["hook_active"] = BebopHook::hook_active;
    j["hook_speed"] = BebopHook::hook_speed;
    j["aim_smoothness"] = BebopHook::aim_smoothness;
    j["enable_delay"] = BebopHook::enable_delay;
    j["hook_activation_key"] = BebopHook::hookActivationKey;

    j["movement"]["script_enabled"] = config.script_enabled;
    j["movement"]["delay_before_space"] = config.delay_before_space;
    j["movement"]["delay_before_ctrl"] = config.delay_before_ctrl;
    j["movement"]["initial_shift_delay"] = config.initial_shift_delay;

    j["sniper"]["aim_active"] = SniperAutoAim::aim_active;
    j["sniper"]["search_radius"] = SniperAutoAim::search_radius;
    j["sniper"]["hp_threshold"] = SniperAutoAim::hp_threshold;

    std::ofstream outputFile("config.json");
    if (!outputFile.is_open()) {
        //std::cout << "Failed to open config.json for writing.\n";
        return;
    }

    outputFile << j.dump(4);
    outputFile.close();

    //std::cout << "Saved config settings to config.json.\n";
}

void RenderConfigMenu() {
    if (ImGui::Button("Load Config")) {
        LoadConfig();
    }
    ImGui::SameLine();
    if (ImGui::Button("Save Config")) {
        SaveConfig();
    }
    ImGui::Text("The config.json is in the game folder");
    ImGui::Text("...\\SteamLibrary\\steamapps\\common\\Deadlock\\game\\bin\\win64");
}