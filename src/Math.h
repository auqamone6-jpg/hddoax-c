#pragma once
#include "SDK.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Math
{
    inline void SinCos(float angle, float& s, float& c)
    {
        s = sinf(angle);
        c = cosf(angle);
    }

    // Build camera-right, camera-up, and camera-forward unit vectors from a FRotator.
    // UE4 uses: X=Forward, Y=Right, Z=Up in world space.
    // Pitch rotates around Y, Yaw rotates around Z, Roll rotates around X.
    inline void RotatorToAxes(const FRotator& rot, FVector& axisX, FVector& axisY, FVector& axisZ)
    {
        float sp, cp, sy, cy, sr, cr;
        SinCos(rot.Pitch * (float)M_PI / 180.f, sp, cp);
        SinCos(rot.Yaw   * (float)M_PI / 180.f, sy, cy);
        SinCos(rot.Roll  * (float)M_PI / 180.f, sr, cr);

        // Forward (X)
        axisX = { cp * cy, cp * sy, sp };

        // Right (Y)
        axisY = {
            sr * sp * cy - cr * sy,
            sr * sp * sy + cr * cy,
            -sr * cp
        };

        // Up (Z)
        axisZ = {
            -(cr * sp * cy + sr * sy),
            -(cr * sp * sy - sr * cy),
            cr * cp
        };
    }

    // Project a world-space point onto screen space.
    // Returns true if the point is in front of the camera.
    // screenW / screenH are the viewport dimensions.
    inline bool WorldToScreen(const FVector& worldPos, const FMinimalViewInfo& pov,
                               float screenW, float screenH, FVector2D& screenPos)
    {
        FVector axisX, axisY, axisZ;
        RotatorToAxes(pov.Rotation, axisX, axisY, axisZ);

        FVector delta  = worldPos - pov.Location;
        FVector coords = {
            delta.Dot(axisY),   // screen X axis
            delta.Dot(axisZ),   // screen Y axis
            delta.Dot(axisX)    // depth (forward)
        };

        if (coords.Z < 1.f) return false;

        float halfW  = screenW  * 0.5f;
        float halfH  = screenH  * 0.5f;
        float tanFov = tanf(pov.FOV * (float)M_PI / 360.f);

        screenPos.X = halfW + coords.X * (halfW / tanFov) / coords.Z;
        screenPos.Y = halfH - coords.Y * (halfW / tanFov) / coords.Z;
        return true;
    }

    // Calculate the angle (degrees) from camera rotation to a world point.
    inline FRotator CalcAngleTo(const FVector& from, const FVector& to)
    {
        FVector delta = to - from;
        float dist2D  = sqrtf(delta.X * delta.X + delta.Y * delta.Y);

        FRotator angles{};
        angles.Pitch = -atan2f(delta.Z, dist2D) * 180.f / (float)M_PI;
        angles.Yaw   =  atan2f(delta.Y, delta.X) * 180.f / (float)M_PI;
        angles.Roll  = 0.f;
        return angles;
    }

    // Normalize a yaw/pitch angle to [-180, 180].
    inline float NormalizeAngle(float a)
    {
        while (a > 180.f)  a -= 360.f;
        while (a < -180.f) a += 360.f;
        return a;
    }

    inline float Clamp(float v, float lo, float hi)
    {
        if (v < lo) return lo;
        if (v > hi) return hi;
        return v;
    }

    // Field-of-view check: is the angle to `target` within `fovDeg` from `currentRot`?
    inline bool InFOV(const FRotator& currentRot, const FVector& cameraPos,
                      const FVector& targetPos, float fovDeg)
    {
        FRotator needed = CalcAngleTo(cameraPos, targetPos);
        float dp = NormalizeAngle(needed.Pitch - currentRot.Pitch);
        float dy = NormalizeAngle(needed.Yaw   - currentRot.Yaw);
        return sqrtf(dp*dp + dy*dy) <= fovDeg;
    }

    // Cross-hair distance on screen (pixels) for target sorting.
    inline float CrosshairDist(const FVector2D& screenPos, float screenW, float screenH)
    {
        float dx = screenPos.X - screenW * 0.5f;
        float dy = screenPos.Y - screenH * 0.5f;
        return sqrtf(dx*dx + dy*dy);
    }
}
