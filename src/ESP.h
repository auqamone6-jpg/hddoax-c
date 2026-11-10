#pragma once
#include "GameReader.h"
#include "Math.h"
#include "Settings.h"
#include <imgui.h>

class ESP
{
public:
    static void Render(const GameReader::GameState& state,
                       float screenW, float screenH);

private:
    static void DrawPlayer(ImDrawList* dl, const PlayerInfo& p,
                           const FMinimalViewInfo& pov,
                           float screenW, float screenH,
                           bool isEnemy);

    static void DrawBox2D(ImDrawList* dl, const FVector2D& top, const FVector2D& bot,
                          float boxW, ImU32 col, ImU32 shadow);

    static void DrawHealthBar(ImDrawList* dl, const FVector2D& top, const FVector2D& bot,
                              float boxW, float hp, float maxHp, float shield, float maxShield);

    static void DrawSkeleton(ImDrawList* dl, const PlayerInfo& p,
                             const FMinimalViewInfo& pov,
                             float screenW, float screenH, ImU32 col);

    static void DrawName(ImDrawList* dl, const PlayerInfo& p,
                         const FVector2D& topScreen, bool isEnemy);

    static void DrawDistance(ImDrawList* dl, const FVector2D& botScreen,
                             float distMetres);

    static ImU32 HealthColor(float hp, float maxHp);
};
