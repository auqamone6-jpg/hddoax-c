#pragma once
#include "GameReader.h"
#include "Math.h"
#include "Settings.h"

class Aimbot
{
public:
    // Run aimbot logic for the current frame.
    // screenW/screenH for FOV circle and crosshair distance.
    static void Update(const GameReader::GameState& state, float screenW, float screenH);

private:
    static const PlayerInfo* SelectTarget(const GameReader::GameState& state,
                                          float screenW, float screenH);

    static void AimAt(const FVector& targetPos, const FMinimalViewInfo& pov);

    static void MoveMouse(float dx, float dy);
};
