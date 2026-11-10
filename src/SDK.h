#pragma once
#include <cstdint>
#include <cmath>
#include "Memory.h"
#include "Offsets.h"

// ---- Math types ----

struct FVector
{
    float X = 0.f, Y = 0.f, Z = 0.f;

    FVector operator+(const FVector& o) const { return { X+o.X, Y+o.Y, Z+o.Z }; }
    FVector operator-(const FVector& o) const { return { X-o.X, Y-o.Y, Z-o.Z }; }
    FVector operator*(float s)          const { return { X*s, Y*s, Z*s }; }

    float Dot(const FVector& o)  const { return X*o.X + Y*o.Y + Z*o.Z; }
    float Length()               const { return sqrtf(X*X + Y*Y + Z*Z); }
    float Dist(const FVector& o) const { return (*this - o).Length(); }
};

struct FVector2D
{
    float X = 0.f, Y = 0.f;
};

struct FRotator
{
    float Pitch = 0.f, Yaw = 0.f, Roll = 0.f;
};

struct FQuat
{
    float X = 0.f, Y = 0.f, Z = 0.f, W = 1.f;
};

struct FTransform
{
    FQuat   Rotation;       // +0x00  (16 bytes)
    FVector Translation;    // +0x10  (12 bytes)
    float   _pad0;          // +0x1C
    FVector Scale3D;        // +0x20  (12 bytes)
    float   _pad1;          // +0x2C
    // total 0x30 bytes

    // Rotate a vector by this transform's rotation quaternion and add Translation.
    FVector TransformPosition(const FVector& v) const
    {
        // Quat rotation: v' = q * v * q^-1  (expanded for unit quaternion)
        float qx = Rotation.X, qy = Rotation.Y, qz = Rotation.Z, qw = Rotation.W;
        float tx = 2.f * (qy * v.Z - qz * v.Y);
        float ty = 2.f * (qz * v.X - qx * v.Z);
        float tz = 2.f * (qx * v.Y - qy * v.X);
        FVector rotated{
            v.X + qw * tx + qy * tz - qz * ty,
            v.Y + qw * ty + qz * tx - qx * tz,
            v.Z + qw * tz + qx * ty - qy * tx
        };
        return rotated + Translation;
    }
};

template<typename T>
struct TArray
{
    uintptr_t Data  = 0; // pointer inside target process
    int32_t   Count = 0;
    int32_t   Max   = 0;

    bool IsValid() const { return Data != 0 && Count > 0 && Count <= 65535; }

    T ReadAt(int index) const
    {
        if (index < 0 || index >= Count) return T{};
        return Memory::Read<T>(Data + static_cast<uintptr_t>(index) * sizeof(T));
    }
};

// ---- UObject (minimal) ----
struct UObject
{
    uintptr_t Address = 0;
    bool IsValid() const { return Memory::IsValidPtr(Address); }

    uintptr_t VTable()      const { return Memory::ReadPtr(Address + Offsets::Object_VTable); }
    int32_t   InternalIdx() const { return Memory::Read<int32_t>(Address + Offsets::Object_InternalIdx); }
    uintptr_t ClassPtr()    const { return Memory::ReadPtr(Address + Offsets::Object_Class); }
    // FName name: ComparisonIndex = int32 at +0x018, Number = int32 at +0x01C
    int32_t   NameIndex()   const { return Memory::Read<int32_t>(Address + Offsets::Object_Name); }
};

// ---- GObjects: FUObjectArray chunked ----
// In UE4 4.26, GObjects points to a FUObjectArray that has:
//   +0x000  FUObjectItem** Objects  (array of chunk pointers)
//   +0x008  FUObjectItem*  PreAllocated
//   +0x010  int32 MaxElements
//   +0x014  int32 NumElements
//   +0x018  int32 MaxChunks
//   +0x01C  int32 NumChunks
// Each FUObjectItem is 0x18 bytes: UObject* at +0
// Chunk size = 65536 elements = chunk indices
struct GObjectsWrapper
{
    uintptr_t ArrayBase = 0; // base address of FUObjectArray inside target

    static constexpr int ChunkSize = 65536;
    static constexpr int ItemSize  = 0x18;

    uintptr_t GetObjectByIndex(int index) const
    {
        if (index < 0) return 0;
        int chunkIndex  = index / ChunkSize;
        int chunkOffset = index % ChunkSize;

        uintptr_t chunksBase = Memory::ReadPtr(ArrayBase);          // FUObjectItem**
        uintptr_t chunk      = Memory::ReadPtr(chunksBase + static_cast<uintptr_t>(chunkIndex) * 8);
        if (!chunk) return 0;
        return Memory::ReadPtr(chunk + static_cast<uintptr_t>(chunkOffset) * ItemSize);
    }

    int NumElements() const { return Memory::Read<int32_t>(ArrayBase + 0x14); }
};

// ---- FMinimalViewInfo ----
struct FMinimalViewInfo
{
    FVector  Location;  // +0x000
    FRotator Rotation;  // +0x00C
    float    FOV;       // +0x018
    // rest not needed
};

// ---- Player data snapshot ----
struct PlayerInfo
{
    uintptr_t PawnAddr   = 0;
    uintptr_t StateAddr  = 0;
    uintptr_t MeshAddr   = 0;

    FVector  RootPos{};
    float    Health      = 0.f;
    float    MaxHealth   = 0.f;
    float    Shield      = 0.f;
    float    MaxShield   = 0.f;
    uint8_t  TeamIndex   = 255;
    bool     bIsBot      = false;
    bool     bIsDBNO     = false;
    bool     bIsDead     = false;

    std::wstring PlayerName;

    // 17 key bones for skeleton + aimbot
    FVector Bones[18]{};

    bool IsValid()   const { return PawnAddr != 0 && !bIsDead; }
    bool IsEnemy(uint8_t localTeam) const { return TeamIndex != localTeam && TeamIndex != 255; }
};

// ---- Shorthand read helpers ----
namespace GameRead
{
    inline FVector ReadFVector(uintptr_t addr)
    {
        return Memory::Read<FVector>(addr);
    }

    inline FRotator ReadFRotator(uintptr_t addr)
    {
        return Memory::Read<FRotator>(addr);
    }

    inline FTransform ReadFTransform(uintptr_t addr)
    {
        return Memory::Read<FTransform>(addr);
    }

    // Read world position from a USceneComponent (RelativeLocation = world for un-attached pawns)
    inline FVector GetActorRootPos(uintptr_t actorAddr)
    {
        uintptr_t root = Memory::ReadPtr(actorAddr + Offsets::Actor_RootComponent);
        if (!Memory::IsValidPtr(root)) return {};
        return ReadFVector(root + Offsets::SceneComp_RelativeLocation);
    }

    // Read a bone world position:
    //   - CachedComponentSpaceTransforms gives component-local positions
    //   - We reconstruct world by adding the mesh component's root-relative location
    //     (approximation that works well for un-rotated static poses)
    inline FVector GetBoneWorldPos(uintptr_t meshAddr, int boneIndex, const FVector& actorWorldPos)
    {
        // TArray<FTransform> at +0x750
        uintptr_t dataPtr = Memory::ReadPtr(meshAddr + Offsets::SkeletalMesh_CachedBoneTransforms);
        int32_t   count   = Memory::Read<int32_t>(meshAddr + Offsets::SkeletalMesh_CachedBoneTransforms + 0x8);

        if (!Memory::IsValidPtr(dataPtr) || boneIndex < 0 || boneIndex >= count)
            return actorWorldPos;

        FTransform bone = Memory::Read<FTransform>(dataPtr + static_cast<uintptr_t>(boneIndex) * sizeof(FTransform));

        // Mesh component RelativeLocation (offset from capsule center, usually (0,0,-90))
        FVector meshRelLoc = ReadFVector(meshAddr + Offsets::SceneComp_RelativeLocation);

        // World = actor root + mesh relative offset + bone local translation
        // (ignoring mesh rotation for simplicity; works well enough for head/spine bones in standing pose)
        return actorWorldPos + meshRelLoc + bone.Translation;
    }
}
