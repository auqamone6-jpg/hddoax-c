#include "Menu.h"

void Menu::Render()
{
    if (!g_settings.menu_open) return;

    ImGui::SetNextWindowSize(ImVec2(420, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(60, 60), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse;
    ImGui::Begin("Fortnite Utility  |  18.40", nullptr, flags);

    if (ImGui::BeginTabBar("MainTabs"))
    {
        if (ImGui::BeginTabItem("ESP"))       { TabESP();    ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Aimbot"))    { TabAimbot(); ImGui::EndTabItem(); }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void Menu::TabESP()
{
    ImGui::Checkbox("Enable ESP",         &g_settings.esp_enabled);
    if (!g_settings.esp_enabled) ImGui::BeginDisabled();

    ImGui::SeparatorText("Elements");
    ImGui::Checkbox("Boxes",              &g_settings.esp_boxes);
    ImGui::SameLine();
    ImGui::Checkbox("Skeleton",           &g_settings.esp_skeleton);
    ImGui::SameLine();
    ImGui::Checkbox("Names",              &g_settings.esp_names);

    ImGui::Checkbox("Health Bar",         &g_settings.esp_health_bar);
    ImGui::SameLine();
    ImGui::Checkbox("Shield Bar",         &g_settings.esp_shield_bar);
    ImGui::SameLine();
    ImGui::Checkbox("Distance",           &g_settings.esp_distance);

    ImGui::Checkbox("DBNO Indicator",     &g_settings.esp_dbno_indicator);

    ImGui::SeparatorText("Box Color  (Enemy)");
    float col[4] = { g_settings.esp_box_r, g_settings.esp_box_g,
                     g_settings.esp_box_b, g_settings.esp_box_a };
    if (ImGui::ColorEdit4("##BoxCol", col, ImGuiColorEditFlags_NoLabel))
    {
        g_settings.esp_box_r = col[0];
        g_settings.esp_box_g = col[1];
        g_settings.esp_box_b = col[2];
        g_settings.esp_box_a = col[3];
    }

    ImGui::SeparatorText("Filters");
    ImGui::SliderFloat("Max Distance (m)", &g_settings.esp_max_distance, 50.f, 1000.f, "%.0f m");

    if (!g_settings.esp_enabled) ImGui::EndDisabled();
}

void Menu::TabAimbot()
{
    ImGui::Checkbox("Enable Aimbot",      &g_settings.aim_enabled);
    if (!g_settings.aim_enabled) ImGui::BeginDisabled();

    ImGui::SeparatorText("Activation");
    ImGui::Checkbox("Hold to Aim (RMB)", &g_settings.aim_hold_key);

    ImGui::SeparatorText("Target");
    const char* boneItems[] = { "Head", "Neck", "Chest (Spine03)" };
    static int boneIdx = 0;
    if (ImGui::Combo("Bone##AimBone", &boneIdx, boneItems, 3))
    {
        if (boneIdx == 0) g_settings.aim_bone = 66;      // head
        else if (boneIdx == 1) g_settings.aim_bone = 65; // neck
        else g_settings.aim_bone = 3;                    // spine03
    }

    ImGui::Checkbox("Enemies Only",        &g_settings.aim_ignore_enemies_only);
    ImGui::SameLine();
    ImGui::Checkbox("Ignore DBNO",         &g_settings.aim_ignore_dbno);

    ImGui::SeparatorText("Behaviour");
    ImGui::SliderFloat("FOV (deg)",        &g_settings.aim_fov,    1.f,  90.f, "%.1f");
    ImGui::SliderFloat("Smooth",           &g_settings.aim_smooth, 1.f,  30.f, "%.1f");
    ImGui::Checkbox("Draw FOV Circle",     &g_settings.aim_draw_fov);

    if (!g_settings.aim_enabled) ImGui::EndDisabled();
}
