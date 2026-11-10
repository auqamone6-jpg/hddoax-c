#pragma once

struct Settings
{
    // ---- ESP ----
    bool     esp_enabled         = true;
    bool     esp_boxes           = true;
    bool     esp_skeleton        = true;
    bool     esp_names           = true;
    bool     esp_health_bar      = true;
    bool     esp_shield_bar      = true;
    bool     esp_distance        = true;
    bool     esp_dbno_indicator  = true;
    float    esp_max_distance    = 400.f;   // metres (UU/100)
    float    esp_box_r           = 1.0f;
    float    esp_box_g           = 0.0f;
    float    esp_box_b           = 0.0f;
    float    esp_box_a           = 1.0f;

    // ---- Aimbot ----
    bool     aim_enabled         = false;
    bool     aim_hold_key        = true;    // true = hold RMB, false = toggle
    int      aim_key             = VK_RBUTTON;
    int      aim_bone            = 66;      // BoneIndex::Head by default
    float    aim_fov             = 15.f;    // degrees
    float    aim_smooth          = 8.f;     // higher = slower/smoother
    bool     aim_draw_fov        = true;
    bool     aim_ignore_dbno     = true;
    bool     aim_ignore_enemies_only = true;

    // ---- Overlay ----
    bool     menu_open           = false;
    float    overlay_fps         = 144.f;
};

inline Settings g_settings;
