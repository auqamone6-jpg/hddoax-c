#include "Aimbot.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cmath>
#include <limits>

void Aimbot::Update(const GameReader::GameState& state, float screenW, float screenH)
{
    if (!g_settings.aim_enabled || !state.bValid) return;

    // Check activation key
    bool keyDown = (GetAsyncKeyState(g_settings.aim_key) & 0x8000) != 0;
    if (g_settings.aim_hold_key && !keyDown) return;

    // Don't aim while menu is open
    if (g_settings.menu_open) return;

    const PlayerInfo* target = SelectTarget(state, screenW, screenH);
    if (!target) return;

    // Choose the aim bone slot: 5 = Head, 3 = Spine03 ("chest")
    FVector aimPos{};
    if (g_settings.aim_bone == 66) // head
        aimPos = target->Bones[5];
    else
        aimPos = target->Bones[3]; // spine03

    AimAt(aimPos, state.CameraPOV);
}

const PlayerInfo* Aimbot::SelectTarget(const GameReader::GameState& state,
                                        float screenW, float screenH)
{
    const PlayerInfo* best   = nullptr;
    float             bestDist = std::numeric_limits<float>::max();

    for (const auto& p : state.Players)
    {
        if (!p.IsValid()) continue;
        if (g_settings.aim_ignore_dbno && p.bIsDBNO) continue;
        if (g_settings.aim_ignore_enemies_only && !p.IsEnemy(state.LocalTeam)) continue;

        // Check world-distance limit
        float worldDist = p.RootPos.Dist(state.CameraPOV.Location) / 100.f;
        if (worldDist > g_settings.esp_max_distance) continue;

        // Project aim bone to screen for crosshair distance
        FVector aimPos = (g_settings.aim_bone == 66) ? p.Bones[5] : p.Bones[3];
        FVector2D screen{};
        if (!Math::WorldToScreen(aimPos, state.CameraPOV, screenW, screenH, screen)) continue;

        // FOV check (degrees)
        if (!Math::InFOV(state.CameraPOV.Rotation, state.CameraPOV.Location,
                          aimPos, g_settings.aim_fov)) continue;

        // Sort by crosshair distance
        float crossDist = Math::CrosshairDist(screen, screenW, screenH);
        if (crossDist < bestDist)
        {
            bestDist = crossDist;
            best     = &p;
        }
    }

    return best;
}

void Aimbot::AimAt(const FVector& targetPos, const FMinimalViewInfo& pov)
{
    FRotator needed = Math::CalcAngleTo(pov.Location, targetPos);

    float dPitch = Math::NormalizeAngle(needed.Pitch - pov.Rotation.Pitch);
    float dYaw   = Math::NormalizeAngle(needed.Yaw   - pov.Rotation.Yaw);

    // Apply smoothing: divide the full delta by smooth factor
    float smooth = Math::Clamp(g_settings.aim_smooth, 1.f, 50.f);
    dPitch /= smooth;
    dYaw   /= smooth;

    MoveMouse(dYaw, dPitch);
}

void Aimbot::MoveMouse(float dx, float dy)
{
    // Convert rotation delta (degrees) to mouse counts.
    // Fortnite default sensitivity ≈ 0.07 (adjust if needed; exposed as a config option later).
    constexpr float sensitivity = 0.07f;
    constexpr float yawPerCount  = 0.0066f; // degrees per count at sens=0.07 (empirical)

    if (fabsf(dx) < 0.01f && fabsf(dy) < 0.01f) return;

    INPUT input{};
    input.type           = INPUT_MOUSE;
    input.mi.dwFlags     = MOUSEEVENTF_MOVE;
    input.mi.dx          = static_cast<LONG>(dx / yawPerCount);
    input.mi.dy          = static_cast<LONG>(dy / yawPerCount);
    SendInput(1, &input, sizeof(INPUT));
}
