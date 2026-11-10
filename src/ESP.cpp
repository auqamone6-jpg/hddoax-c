#include "ESP.h"
#include <imgui.h>
#include <cstdio>
#include <cwchar>

static ImU32 ToImU32(float r, float g, float b, float a = 1.f)
{
    return IM_COL32(
        static_cast<int>(r * 255.f),
        static_cast<int>(g * 255.f),
        static_cast<int>(b * 255.f),
        static_cast<int>(a * 255.f)
    );
}

void ESP::Render(const GameReader::GameState& state, float screenW, float screenH)
{
    if (!g_settings.esp_enabled || !state.bValid) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();

    for (const auto& p : state.Players)
    {
        if (!p.IsValid()) continue;

        float distUU = p.RootPos.Dist(state.CameraPOV.Location);
        float distM  = distUU / 100.f; // Unreal units → metres
        if (distM > g_settings.esp_max_distance) continue;

        bool isEnemy = p.IsEnemy(state.LocalTeam);
        DrawPlayer(dl, p, state.CameraPOV, screenW, screenH, isEnemy);
    }

    // ---- Aimbot FOV circle ----
    if (g_settings.aim_enabled && g_settings.aim_draw_fov)
    {
        float radius = (screenW / 2.f) / tanf(state.CameraPOV.FOV * (float)M_PI / 360.f)
                       * tanf(g_settings.aim_fov * (float)M_PI / 360.f);
        dl->AddCircle(ImVec2(screenW * 0.5f, screenH * 0.5f),
                      radius, IM_COL32(255, 255, 255, 80), 64, 1.5f);
    }
}

void ESP::DrawPlayer(ImDrawList* dl, const PlayerInfo& p,
                     const FMinimalViewInfo& pov,
                     float screenW, float screenH, bool isEnemy)
{
    // We use the head and feet bones for box sizing.
    // Head = Bones[5] (slot index), Feet mid = Bones[0] (Pelvis)
    // Box top = head screen pos, box bottom = feet (root pos)

    FVector headPos = p.Bones[5]; // Head
    FVector feetPos = p.RootPos;
    // Adjust feet to slightly below the pawn root
    feetPos.Z -= 90.f;

    FVector2D headScreen{}, feetScreen{};
    bool headOk = Math::WorldToScreen(headPos, pov, screenW, screenH, headScreen);
    bool feetOk = Math::WorldToScreen(feetPos, pov, screenW, screenH, feetScreen);

    if (!headOk || !feetOk) return;

    // Clamp to screen bounds
    if (headScreen.Y < 0.f || headScreen.Y > screenH) return;
    if (feetScreen.Y < 0.f || feetScreen.Y > screenH) return;

    float boxH = feetScreen.Y - headScreen.Y;
    if (boxH < 2.f) return;
    float boxW = boxH * 0.4f; // typical aspect ratio for a standing player

    ImU32 colMain   = isEnemy
        ? ToImU32(g_settings.esp_box_r, g_settings.esp_box_g, g_settings.esp_box_b, g_settings.esp_box_a)
        : IM_COL32(0, 200, 100, 220);
    ImU32 colShadow = IM_COL32(0, 0, 0, 160);

    // ---- Box ----
    if (g_settings.esp_boxes)
        DrawBox2D(dl, headScreen, feetScreen, boxW, colMain, colShadow);

    // ---- Health / Shield bars ----
    if (g_settings.esp_health_bar || g_settings.esp_shield_bar)
        DrawHealthBar(dl, headScreen, feetScreen, boxW, p.Health, p.MaxHealth, p.Shield, p.MaxShield);

    // ---- Skeleton ----
    if (g_settings.esp_skeleton)
        DrawSkeleton(dl, p, pov, screenW, screenH, colMain);

    // ---- Name ----
    if (g_settings.esp_names)
        DrawName(dl, p, headScreen, isEnemy);

    // ---- Distance ----
    if (g_settings.esp_distance)
    {
        float distM = p.RootPos.Dist(pov.Location) / 100.f;
        DrawDistance(dl, feetScreen, distM);
    }

    // ---- DBNO indicator ----
    if (g_settings.esp_dbno_indicator && p.bIsDBNO)
    {
        dl->AddText(ImVec2(headScreen.X - 15.f, headScreen.Y - 28.f),
                    IM_COL32(255, 180, 0, 255), "[DBNO]");
    }
}

void ESP::DrawBox2D(ImDrawList* dl, const FVector2D& top, const FVector2D& bot,
                    float boxW, ImU32 col, ImU32 shadow)
{
    float left  = top.X - boxW * 0.5f;
    float right = top.X + boxW * 0.5f;

    // Shadow
    dl->AddRect(ImVec2(left - 1, top.Y - 1), ImVec2(right + 1, bot.Y + 1), shadow, 0.f, 0, 3.f);
    // Main
    dl->AddRect(ImVec2(left, top.Y), ImVec2(right, bot.Y), col, 0.f, 0, 1.5f);
}

void ESP::DrawHealthBar(ImDrawList* dl, const FVector2D& top, const FVector2D& bot,
                        float boxW, float hp, float maxHp, float shield, float maxShield)
{
    if (maxHp <= 0.f) maxHp = 100.f;

    float left  = top.X - boxW * 0.5f;
    float right = top.X + boxW * 0.5f;
    float boxH  = bot.Y - top.Y;

    if (g_settings.esp_health_bar)
    {
        float ratio     = Math::Clamp(hp / maxHp, 0.f, 1.f);
        float barLeft   = left - 6.f;
        float barRight  = barLeft + 3.f;
        float barTop    = top.Y;
        float barBot    = bot.Y;
        float fillBot   = barBot;
        float fillTop   = barBot - boxH * ratio;

        dl->AddRectFilled(ImVec2(barLeft, barTop), ImVec2(barRight, barBot), IM_COL32(0,0,0,160));
        dl->AddRectFilled(ImVec2(barLeft, fillTop), ImVec2(barRight, fillBot), HealthColor(hp, maxHp));
        dl->AddRect(ImVec2(barLeft, barTop), ImVec2(barRight, barBot), IM_COL32(0,0,0,255), 0.f, 0, 1.f);
    }

    if (g_settings.esp_shield_bar && maxShield > 0.f)
    {
        float ratio     = Math::Clamp(shield / maxShield, 0.f, 1.f);
        float barLeft   = left - 11.f;
        float barRight  = barLeft + 3.f;
        float barTop    = top.Y;
        float barBot    = bot.Y;
        float fillBot   = barBot;
        float fillTop   = barBot - boxH * ratio;

        dl->AddRectFilled(ImVec2(barLeft, barTop), ImVec2(barRight, barBot), IM_COL32(0,0,0,160));
        dl->AddRectFilled(ImVec2(barLeft, fillTop), ImVec2(barRight, fillBot), IM_COL32(50, 150, 255, 220));
        dl->AddRect(ImVec2(barLeft, barTop), ImVec2(barRight, barBot), IM_COL32(0,0,0,255), 0.f, 0, 1.f);
    }
}

void ESP::DrawSkeleton(ImDrawList* dl, const PlayerInfo& p,
                       const FMinimalViewInfo& pov,
                       float screenW, float screenH, ImU32 col)
{
    // Bone slot indices:
    // 0=Pelvis, 1=Spine01, 2=Spine02, 3=Spine03, 4=Neck, 5=Head
    // 6=UArmL, 7=LArmL, 8=HandL,  9=UArmR, 10=LArmR, 11=HandR
    // 12=ThighL, 13=CalfL, 14=FootL, 15=ThighR, 16=CalfR, 17=FootR
    static const int connections[][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5},   // spine → head
        {3, 6}, {6, 7}, {7, 8},                     // left arm
        {3, 9}, {9, 10}, {10, 11},                  // right arm
        {0, 12}, {12, 13}, {13, 14},                // left leg
        {0, 15}, {15, 16}, {16, 17}                 // right leg
    };

    ImU32 lineCol = IM_COL32(
        (col & 0xFF),
        ((col >> 8) & 0xFF),
        ((col >> 16) & 0xFF),
        180
    );

    for (const auto& conn : connections)
    {
        FVector2D a{}, b{};
        if (Math::WorldToScreen(p.Bones[conn[0]], pov, screenW, screenH, a) &&
            Math::WorldToScreen(p.Bones[conn[1]], pov, screenW, screenH, b))
        {
            dl->AddLine(ImVec2(a.X, a.Y), ImVec2(b.X, b.Y), lineCol, 1.5f);
        }
    }
}

void ESP::DrawName(ImDrawList* dl, const PlayerInfo& p,
                   const FVector2D& topScreen, bool isEnemy)
{
    if (p.PlayerName.empty()) return;

    // Convert wstring to UTF-8 for ImGui
    char nameBuf[128]{};
    size_t len = p.PlayerName.size();
    if (len >= sizeof(nameBuf)) len = sizeof(nameBuf) - 1;
    for (size_t i = 0; i < len; ++i)
        nameBuf[i] = (p.PlayerName[i] < 128) ? static_cast<char>(p.PlayerName[i]) : '?';
    nameBuf[len] = '\0';

    ImVec2 textSize = ImGui::CalcTextSize(nameBuf);
    float  textX    = topScreen.X - textSize.x * 0.5f;
    float  textY    = topScreen.Y - textSize.y - 2.f;

    ImU32 col = isEnemy ? IM_COL32(255, 80, 80, 255) : IM_COL32(80, 255, 80, 255);
    dl->AddText(ImVec2(textX + 1, textY + 1), IM_COL32(0, 0, 0, 180), nameBuf); // shadow
    dl->AddText(ImVec2(textX, textY), col, nameBuf);
}

void ESP::DrawDistance(ImDrawList* dl, const FVector2D& botScreen, float distMetres)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%.0fm", distMetres);
    ImVec2 textSize = ImGui::CalcTextSize(buf);
    float  textX    = botScreen.X - textSize.x * 0.5f;
    float  textY    = botScreen.Y + 2.f;
    dl->AddText(ImVec2(textX + 1, textY + 1), IM_COL32(0, 0, 0, 180), buf);
    dl->AddText(ImVec2(textX, textY), IM_COL32(220, 220, 220, 220), buf);
}

ImU32 ESP::HealthColor(float hp, float maxHp)
{
    float ratio = (maxHp > 0.f) ? hp / maxHp : 0.f;
    if (ratio > 0.5f)
    {
        // Green → Yellow
        float t = (ratio - 0.5f) * 2.f;
        return IM_COL32(static_cast<int>((1.f - t) * 255), 255, 0, 220);
    }
    // Yellow → Red
    float t = ratio * 2.f;
    return IM_COL32(255, static_cast<int>(t * 255), 0, 220);
}
