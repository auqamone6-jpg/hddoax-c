#include "GameReader.h"
#include "Offsets.h"
#include <algorithm>

bool GameReader::Update()
{
    m_state.bValid   = false;
    m_state.Players.clear();

    if (!ReadGlobals()) return false;

    // ---- Read local player controller ----
    // Path: GWorld → OwningGameInstance → LocalPlayers[0] → PlayerController
    uintptr_t gameInstance = Memory::ReadPtr(m_worldCache + Offsets::World_OwningGameInstance);
    if (!Memory::IsValidPtr(gameInstance)) return false;

    uintptr_t localPlayersData  = Memory::ReadPtr(gameInstance + Offsets::GameInstance_LocalPlayers);
    int32_t   localPlayersCount = Memory::Read<int32_t>(gameInstance + Offsets::GameInstance_LocalPlayers + 0x8);
    if (!Memory::IsValidPtr(localPlayersData) || localPlayersCount <= 0) return false;

    uintptr_t localPlayer = Memory::ReadPtr(localPlayersData); // [0]
    if (!Memory::IsValidPtr(localPlayer)) return false;

    uintptr_t localPC = Memory::ReadPtr(localPlayer + Offsets::LocalPlayer_PlayerController);
    if (!Memory::IsValidPtr(localPC)) return false;

    // ---- Read camera ----
    if (!ReadCamera(localPC)) return false;

    // ---- Get local pawn for team detection ----
    uintptr_t localPawn = Memory::ReadPtr(localPC + Offsets::PlayerController_AcknowledgedPawn);
    m_state.LocalPawn = localPawn;

    if (Memory::IsValidPtr(localPawn))
    {
        uintptr_t localState = Memory::ReadPtr(localPawn + Offsets::Pawn_PlayerState);
        if (Memory::IsValidPtr(localState))
            m_state.LocalTeam = Memory::Read<uint8_t>(localState + Offsets::FortState_TeamIndex);
    }

    // ---- Read all players from game state ----
    if (!ReadPlayers(m_gameStateCache)) return false;

    m_state.bValid = true;
    return true;
}

bool GameReader::ReadGlobals()
{
    // UWorld* world = *reinterpret_cast<UWorld**>(base + GWorld)
    uintptr_t worldPtr = Memory::BaseAddress + Offsets::GWorld;
    uintptr_t world    = Memory::ReadPtr(worldPtr);
    if (!Memory::IsValidPtr(world)) return false;
    m_worldCache = world;

    uintptr_t gameState = Memory::ReadPtr(world + Offsets::World_GameState);
    if (!Memory::IsValidPtr(gameState)) return false;
    m_gameStateCache = gameState;

    return true;
}

bool GameReader::ReadCamera(uintptr_t localPC)
{
    uintptr_t cameraMgr = Memory::ReadPtr(localPC + Offsets::PlayerController_PlayerCameraManager);
    if (!Memory::IsValidPtr(cameraMgr)) return false;

    // CameraCache is a struct (not pointer), starts at +0x290
    // FCameraCacheEntry: { float Timestamp (+0), FMinimalViewInfo POV (+0x10) }
    uintptr_t cacheAddr = cameraMgr + Offsets::CameraManager_CameraCache;
    uintptr_t povAddr   = cacheAddr + Offsets::CameraCache_POV;

    m_state.CameraPOV.Location = Memory::Read<FVector>(povAddr + Offsets::POV_Location);
    m_state.CameraPOV.Rotation = Memory::Read<FRotator>(povAddr + Offsets::POV_Rotation);
    m_state.CameraPOV.FOV      = Memory::Read<float>(povAddr + Offsets::POV_FOV);

    if (m_state.CameraPOV.FOV < 1.f || m_state.CameraPOV.FOV > 180.f)
        m_state.CameraPOV.FOV = 80.f;

    return true;
}

bool GameReader::ReadPlayers(uintptr_t gameState)
{
    // PlayerArray = TArray<APlayerState*>
    uintptr_t arrData  = Memory::ReadPtr(gameState + Offsets::GameState_PlayerArray);
    int32_t   arrCount = Memory::Read<int32_t>(gameState + Offsets::GameState_PlayerArray + 0x8);

    if (!Memory::IsValidPtr(arrData) || arrCount <= 0 || arrCount > MaxPlayers)
        return false;

    m_state.Players.reserve(arrCount);

    for (int i = 0; i < arrCount; ++i)
    {
        uintptr_t stateAddr = Memory::ReadPtr(arrData + static_cast<uintptr_t>(i) * 8);
        if (!Memory::IsValidPtr(stateAddr)) continue;

        PlayerInfo info{};
        if (!ReadSinglePlayer(stateAddr, info)) continue;

        // Skip local pawn
        if (info.PawnAddr == m_state.LocalPawn) continue;

        m_state.Players.push_back(info);
    }

    return true;
}

bool GameReader::ReadSinglePlayer(uintptr_t playerStateAddr, PlayerInfo& out)
{
    out.StateAddr = playerStateAddr;

    // Get pawn
    uintptr_t pawnAddr = Memory::ReadPtr(playerStateAddr + Offsets::PlayerState_PawnPrivate);
    if (!Memory::IsValidPtr(pawnAddr)) return false;
    out.PawnAddr = pawnAddr;

    // Team
    out.TeamIndex = Memory::Read<uint8_t>(playerStateAddr + Offsets::FortState_TeamIndex);

    // Player name (FString)
    out.PlayerName = Memory::ReadFString(playerStateAddr + Offsets::PlayerState_PlayerNamePrivate);

    // Dead / DBNO flags
    uint8_t flags0 = Memory::Read<uint8_t>(pawnAddr + Offsets::FortPawn_bIsDying);
    out.bIsDead  = (flags0 & (1 << 3)) != 0;
    uint8_t flags1 = Memory::Read<uint8_t>(pawnAddr + Offsets::FortPawn_bIsDBNO);
    out.bIsDBNO  = (flags1 & (1 << 3)) != 0;

    if (out.bIsDead) return false; // skip dead players

    // World position from root component
    out.RootPos = GameRead::GetActorRootPos(pawnAddr);

    // Health / Shield
    uintptr_t healthSet = Memory::ReadPtr(pawnAddr + Offsets::FortPawn_HealthSet);
    if (Memory::IsValidPtr(healthSet))
    {
        out.Health    = ReadAttributeCurrentValue(healthSet + Offsets::HealthSet_Health);
        out.MaxHealth = ReadAttributeCurrentValue(healthSet + Offsets::HealthSet_MaxHealth);
        out.Shield    = ReadAttributeCurrentValue(healthSet + Offsets::HealthSet_CurrentShield);
        out.MaxShield = ReadAttributeCurrentValue(healthSet + Offsets::HealthSet_Shield);
        // Clamp to reasonable ranges
        out.Health    = Math::Clamp(out.Health,    0.f, 1000.f);
        out.MaxHealth = Math::Clamp(out.MaxHealth, 1.f, 1000.f);
        out.Shield    = Math::Clamp(out.Shield,    0.f, 1000.f);
        out.MaxShield = Math::Clamp(out.MaxShield, 0.f, 1000.f);
    }
    else
    {
        out.MaxHealth = 100.f;
        out.MaxShield = 100.f;
    }

    // Mesh
    uintptr_t meshAddr = Memory::ReadPtr(pawnAddr + Offsets::Character_Mesh);
    out.MeshAddr = meshAddr;

    // Bones
    if (Memory::IsValidPtr(meshAddr))
        ReadBones(out, out);

    return true;
}

bool GameReader::ReadBones(const PlayerInfo& p, PlayerInfo& out)
{
    if (!Memory::IsValidPtr(p.MeshAddr)) return false;

    uintptr_t dataPtr = Memory::ReadPtr(p.MeshAddr + Offsets::SkeletalMesh_CachedBoneTransforms);
    int32_t   count   = Memory::Read<int32_t>(p.MeshAddr + Offsets::SkeletalMesh_CachedBoneTransforms + 0x8);
    if (!Memory::IsValidPtr(dataPtr) || count <= 0 || count > 500) return false;

    // Mesh RelativeLocation (component-to-capsule offset, e.g. Z=-90)
    FVector meshRelLoc = Memory::Read<FVector>(p.MeshAddr + Offsets::SceneComp_RelativeLocation);

    // Read the specific bone indices we care about
    static const int indices[] = {
        BoneIndex::Pelvis,
        BoneIndex::Spine01, BoneIndex::Spine02, BoneIndex::Spine03,
        BoneIndex::Neck,    BoneIndex::Head,
        BoneIndex::UpperArmL, BoneIndex::LowerArmL, BoneIndex::HandL,
        BoneIndex::UpperArmR, BoneIndex::LowerArmR, BoneIndex::HandR,
        BoneIndex::ThighL, BoneIndex::CalfL, BoneIndex::FootL,
        BoneIndex::ThighR, BoneIndex::CalfR, BoneIndex::FootR
    };

    for (int slot = 0; slot < 18; ++slot)
    {
        int boneIdx = indices[slot];
        if (boneIdx >= count) { out.Bones[slot] = p.RootPos; continue; }

        FTransform bone = Memory::Read<FTransform>(dataPtr + static_cast<uintptr_t>(boneIdx) * sizeof(FTransform));
        // World = actor root + mesh relative offset + bone local translation
        out.Bones[slot] = p.RootPos + meshRelLoc + bone.Translation;
    }

    return true;
}

float GameReader::ReadAttributeCurrentValue(uintptr_t attributeDataAddr) const
{
    // FFortGameplayAttributeData inherits FGameplayAttributeData
    // FGameplayAttributeData: +0x008 BaseValue, +0x00C CurrentValue
    return Memory::Read<float>(attributeDataAddr + Offsets::AttributeData_CurrentValue);
}
