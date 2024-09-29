#include "visuals.h"
#include <chrono>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstring>

std::string read_str_from_memory(uintptr_t address) {
    uintptr_t pointer_to_string = memory::memRead<uintptr_t>(address);
    if (!pointer_to_string) return "";

    const size_t max_length = 1024;
    char buffer[max_length] = { 0 };

    SIZE_T bytesRead;
    if (!ReadProcessMemory(memory::processHandle, reinterpret_cast<LPCVOID>(pointer_to_string), buffer, max_length - 1, &bytesRead) || bytesRead == 0) {
        return "";
    }

    return std::string(buffer);
}

enum HealthDisplayMode {
    HEALTH_NONE = 0,
    HEALTH_BAR,
    HEALTH_NUMBER
};

HealthDisplayMode health_display_mode = HEALTH_BAR;

ImColor LerpColor(const ImColor& col1, const ImColor& col2, float t) {
    return ImColor(
        col1.Value.x + t * (col2.Value.x - col1.Value.x),
        col1.Value.y + t * (col2.Value.y - col1.Value.y),
        col1.Value.z + t * (col2.Value.z - col1.Value.z)
    );
}

const float screen_center_x = GetSystemMetrics(SM_CXSCREEN) / 2.0f;
const float screen_height = static_cast<float>(GetSystemMetrics(SM_CYSCREEN));

std::vector<EntityCache> Visuals1::entityCache;
std::chrono::time_point<std::chrono::steady_clock> Visuals1::lastCacheUpdate = std::chrono::steady_clock::now();
const int Visuals1::cacheUpdateInterval = 1000;

float Visuals1::boxThickness = 1.0f;
float Visuals1::lineThickness = 1.0f;
ImVec4 Visuals1::boxColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
ImVec4 Visuals1::lineColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
bool Visuals1::esp_show_head = false;
bool Visuals1::esp_show_circle = false;
float Visuals1::esp_distance_radius = 2400.0f;
bool Visuals1::esp_box_enabled = true;
bool Visuals1::esp_line_enabled = true;
bool Visuals1::esp_health_enabled = true;
bool Visuals1::show_health = true;
bool Visuals1::show_max_health = true;
bool Visuals1::esp_show_address = false;
bool Visuals1::esp_show_name = false;

void Visuals1::UpdateEntityCache() {
    ULONG_PTR entity_list = get_entity_list();
    int max_ents = get_max_entities();
    entityCache.clear();
    entityCache.reserve(max_ents);

    for (int i = 1; i <= max_ents; ++i) {
        uintptr_t entity = get_base_entity_from_index(i, entity_list);
        if (!entity) continue;

        std::string designer_name = get_designer_name(entity);
        if (designer_name != "player") continue;

        EntityCache cache;
        cache.entityAddress = entity;
        cache.team = memory::memRead<uint8_t>(entity + 0x3eb);
        cache.isValid = true;

        std::ostringstream ss;
        ss << "0x" << std::hex << std::uppercase << entity;
        cache.entityAddressStr = ss.str();

        uintptr_t name_ptr = memory::memRead<uintptr_t>(entity + 0x840);
        if (name_ptr) {
            std::string model_path = read_str_from_memory(name_ptr + 0x830);
            cache.entityName = get_entity_human_name(model_path);
            cache.headBoneIndex = get_bone_head_index(cache.entityName);
        }
        else {
            cache.entityName = "Unknown";
            cache.headBoneIndex = -1;
        }

        entityCache.push_back(cache);
    }
    lastCacheUpdate = std::chrono::steady_clock::now();
}

void Visuals1::PlayerEsp(uint8_t& local_team) {
    uintptr_t local_pawn = memory::memRead<uintptr_t>(memory::baseAddress + 0x20fd718);
    if (!local_pawn) return;

    Vector3 local_player_pos = memory::memRead<Vector3>(local_pawn + offsets::m_vOldOrigin);
    ViewMatrix view_matrix = get_view_matrix();

    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCacheUpdate).count() > cacheUpdateInterval) {
        UpdateEntityCache();
    }

    for (auto& cache : entityCache) {
        if (!cache.isValid || cache.entityAddress == local_pawn || cache.team == local_team) continue;

        uint32_t isVisible = memory::memRead<uint32_t>(cache.entityAddress + 0x30);
        if (isVisible != 1) continue;

        uint32_t health = memory::memRead<uint32_t>(cache.entityAddress + 0x34c);
        uint32_t health_max = memory::memRead<uint32_t>(cache.entityAddress + 0x348);
        if (health <= 0) continue;
        if (health > health_max) health_max = health;

        Vector3 entity_world_pos = memory::memRead<Vector3>(cache.entityAddress + offsets::m_vOldOrigin);
        Vector3 entity_head_pos = (esp_show_head && cache.headBoneIndex != -1)
            ? get_bone_position_by_index(cache.entityAddress, cache.headBoneIndex)
            : entity_world_pos + Vector3(0, 0, 92.0f);

        Vector3 screen_pos_foot = WorldToScreen(view_matrix, entity_world_pos);
        Vector3 screen_pos_head = WorldToScreen(view_matrix, entity_head_pos);
        ImDrawList* draw_list = ImGui::GetForegroundDrawList();

        if (screen_pos_foot.Z < 0.01f || screen_pos_head.Z < 0.01f) continue;
        if (esp_line_enabled) {
            draw_list->AddLine(ImVec2(screen_center_x, screen_height), ImVec2(screen_pos_foot.X, screen_pos_foot.Y), ImColor(lineColor), lineThickness);
        }

        float distance_to_enemy = (local_player_pos - entity_world_pos).Length();
        if (esp_show_circle && distance_to_enemy <= esp_distance_radius) {
            draw_list->AddCircleFilled(ImVec2(screen_pos_head.X, screen_pos_head.Y - 20.0f), 8.0f, ImColor(0, 255, 0));
        }

        float box_height = screen_pos_foot.Y - screen_pos_head.Y;
        float box_width = box_height / 2.0f;
        if (esp_box_enabled) {
            draw_list->AddRect(ImVec2(screen_pos_head.X - box_width / 2, screen_pos_head.Y),
                ImVec2(screen_pos_head.X + box_width / 2, screen_pos_foot.Y), ImColor(boxColor), 0.0f, 0, boxThickness);
        }

        if (esp_show_head) {
            draw_list->AddCircle(ImVec2(screen_pos_head.X, screen_pos_head.Y), 5.0f, ImColor(255, 255, 0), 16, 2.0f);
        }

        if (esp_health_enabled) {
            if (health_display_mode == HEALTH_BAR) {
                float health_percentage = static_cast<float>(health) / static_cast<float>(health_max);
                ImColor health_color = LerpColor(ImColor(255, 0, 0), ImColor(0, 255, 0), health_percentage);
                draw_list->AddRect(ImVec2(screen_pos_head.X - box_width / 2 - 8.0f, screen_pos_head.Y),
                    ImVec2(screen_pos_head.X - box_width / 2 - 4.5f, screen_pos_foot.Y), ImColor(255, 255, 255));
                draw_list->AddRectFilled(ImVec2(screen_pos_head.X - box_width / 2 - 7.5f, screen_pos_foot.Y - box_height * health_percentage),
                    ImVec2(screen_pos_head.X - box_width / 2 - 4.5f, screen_pos_foot.Y), health_color);
            }
            else if (health_display_mode == HEALTH_NUMBER) {
                std::string health_info = std::to_string(health) + "/" + std::to_string(health_max);
                draw_list->AddText(ImVec2(screen_pos_head.X - 25, screen_pos_head.Y), ImColor(255, 255, 255), health_info.c_str());
            }
        }

        if (esp_show_name) {
            draw_list->AddText(ImVec2(screen_pos_head.X - box_width / 2, screen_pos_head.Y - 20.0f), ImColor(255, 255, 255), cache.entityName.c_str());
        }

        if (esp_show_address) {
            draw_list->AddText(ImVec2(screen_pos_head.X - box_width / 2, screen_pos_head.Y - 30.0f), ImColor(255, 255, 255), cache.entityAddressStr.c_str());
        }
    }
}

void Visuals1::RenderSettingsMenu() {
    static bool circle_settings_visible = true;
    if (ImGui::Checkbox("Enable ESP Circle", &esp_show_circle)) {
        circle_settings_visible = esp_show_circle;
    }
    if (circle_settings_visible) {
        ImGui::Indent();
        ImGui::SliderFloat("Circle Distance Radius", &esp_distance_radius, 500.0f, 5000.0f, "Radius: %.0f");
        ImGui::Unindent();
    }

    static bool box_settings_visible = true;
    if (ImGui::Checkbox("Enable ESP Box", &esp_box_enabled)) {
        box_settings_visible = esp_box_enabled;
    }
    if (box_settings_visible) {
        ImGui::Indent();
        ImGui::SliderFloat("Box Thickness", &boxThickness, 1.0f, 10.0f, "Thickness: %.1f");
        ImGui::ColorEdit4("Box Color", (float*)&boxColor);
        ImGui::Unindent();
    }

    static bool line_settings_visible = true;
    if (ImGui::Checkbox("Enable ESP Line", &esp_line_enabled)) {
        line_settings_visible = esp_line_enabled;
    }
    if (line_settings_visible) {
        ImGui::Indent();
        ImGui::SliderFloat("Line Thickness", &lineThickness, 1.0f, 10.0f, "Thickness: %.1f");
        ImGui::ColorEdit4("Line Color", (float*)&lineColor);
        ImGui::Unindent();
    }

    static bool health_settings_visible = true;
    if (ImGui::Checkbox("Enable ESP Health", &esp_health_enabled)) {
        health_settings_visible = esp_health_enabled;
    }
    if (health_settings_visible) {
        ImGui::Indent();
        ImGui::Text("Health Display Mode");
        if (ImGui::RadioButton("Health Bar", health_display_mode == HEALTH_BAR)) {
            health_display_mode = HEALTH_BAR;
        }
        if (ImGui::RadioButton("Health Number", health_display_mode == HEALTH_NUMBER)) {
            health_display_mode = HEALTH_NUMBER;
        }
        ImGui::Unindent();
    }

    static bool name_settings_visible = false;
    if (ImGui::Checkbox("Show Entity Name", &esp_show_name)) {
        name_settings_visible = esp_show_name;
    }
    static bool address_settings_visible = false;
    if (ImGui::Checkbox("Show Entity Address", &esp_show_address)) {
        address_settings_visible = esp_show_address;
    }
    static bool head_settings_visible = false;
    if (ImGui::Checkbox("Show Entity Head", &esp_show_head)) {
        head_settings_visible = esp_show_head;
    }
}
