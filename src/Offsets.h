#pragma once
#include <cstdint>

// Fortnite 4.26.1-18163738 (Chapter 2 Season 8 / 18.40)
// All offsets are relative to FortniteClient-Win64-Shipping.exe base.

namespace Offsets
{
    // ---- Global pointers (relative to module base) ----
    constexpr uintptr_t GObjects        = 0x9CA5878; // FUObjectArray (chunked)
    constexpr uintptr_t GWorld          = 0x9EEF760; // UWorld**
    constexpr uintptr_t GNames          = 0x9F27DC0; // FNamePool**
    constexpr uintptr_t AppendString    = 0x00CC306C; // FName::AppendAnsiString (vtable or func ptr)
    constexpr uintptr_t ProcessEvent    = 0x00D714FC; // UObject::ProcessEvent

    // ---- UEngine ----
    constexpr uintptr_t Engine_GameViewport = 0x780; // UGameViewportClient*

    // ---- UGameViewportClient ----
    constexpr uintptr_t Viewport_World          = 0x078; // UWorld*
    constexpr uintptr_t Viewport_GameInstance   = 0x080; // UGameInstance*

    // ---- UGameInstance ----
    constexpr uintptr_t GameInstance_LocalPlayers = 0x038; // TArray<ULocalPlayer*>

    // ---- ULocalPlayer ----
    constexpr uintptr_t LocalPlayer_PlayerController = 0x030; // APlayerController*

    // ---- UWorld ----
    constexpr uintptr_t World_PersistentLevel   = 0x030; // ULevel*
    constexpr uintptr_t World_GameState         = 0x128; // AGameStateBase*
    constexpr uintptr_t World_OwningGameInstance= 0x188; // UGameInstance*

    // ---- AGameStateBase ----
    constexpr uintptr_t GameState_PlayerArray = 0x238; // TArray<APlayerState*>  (568d)

    // ---- UObject ----
    constexpr uintptr_t Object_VTable     = 0x000; // void**
    constexpr uintptr_t Object_Flags      = 0x008; // EObjectFlags (int32)
    constexpr uintptr_t Object_InternalIdx= 0x00C; // int32
    constexpr uintptr_t Object_Class      = 0x010; // UClass*
    constexpr uintptr_t Object_Name       = 0x018; // FName  (ComparisonIdx:int32, Number:int32)
    constexpr uintptr_t Object_Outer      = 0x020; // UObject*

    // ---- AActor ----
    constexpr uintptr_t Actor_RootComponent = 0x130; // USceneComponent*  (304d)

    // ---- APawn ----
    constexpr uintptr_t Pawn_PlayerState  = 0x240; // APlayerState*  (576d)
    constexpr uintptr_t Pawn_Controller   = 0x258; // AController*   (600d)

    // ---- AController ----
    constexpr uintptr_t Controller_PlayerState = 0x228; // APlayerState*
    constexpr uintptr_t Controller_Pawn        = 0x250; // APawn*

    // ---- APlayerController ----
    constexpr uintptr_t PlayerController_AcknowledgedPawn    = 0x2A0; // APawn*          (672d)
    constexpr uintptr_t PlayerController_PlayerCameraManager = 0x2B8; // APlayerCameraManager* (696d)

    // ---- APlayerCameraManager ----
    constexpr uintptr_t CameraManager_CameraCache = 0x290; // FCameraCacheEntry (656d)

    // FCameraCacheEntry layout (size 0x5F0):
    //   +0x000  float          Timestamp
    //   +0x010  FMinimalViewInfo  POV
    // FMinimalViewInfo layout:
    //   +0x000  FVector  Location
    //   +0x00C  FRotator Rotation
    //   +0x018  float    FOV
    constexpr uintptr_t CameraCache_POV      = 0x010; // FMinimalViewInfo within entry
    constexpr uintptr_t POV_Location         = 0x000; // FVector
    constexpr uintptr_t POV_Rotation         = 0x00C; // FRotator
    constexpr uintptr_t POV_FOV              = 0x018; // float

    // ---- APlayerState ----
    constexpr uintptr_t PlayerState_PawnPrivate      = 0x280; // APawn*   (640d)
    constexpr uintptr_t PlayerState_PlayerNamePrivate= 0x300; // FString  (768d)

    // ---- AFortPlayerStateAthena ----
    constexpr uintptr_t FortState_TeamIndex   = 0xF28; // uint8  (3880d)
    constexpr uintptr_t FortState_KillScore   = 0xF3C; // int32  (3900d)

    // ---- ACharacter ----
    constexpr uintptr_t Character_Mesh              = 0x280; // USkeletalMeshComponent* (640d)
    constexpr uintptr_t Character_CharacterMovement = 0x288; // UCharacterMovementComponent* (648d)
    constexpr uintptr_t Character_CapsuleComponent  = 0x290; // UCapsuleComponent* (656d)

    // ---- AFortPawn ----
    constexpr uintptr_t FortPawn_CurrentWeapon    = 0x5E8; // AFortWeapon*
    constexpr uintptr_t FortPawn_bIsDBNO          = 0x562; // bitfield (bit 3)
    constexpr uintptr_t FortPawn_bIsDying         = 0x548; // bitfield (bit 3)
    constexpr uintptr_t FortPawn_HealthSet         = 0xCE0; // UFortHealthSet* (3296d)
    constexpr uintptr_t FortPawn_AbilitySystem     = 0xD20; // UFortAbilitySystemComponent*

    // ---- UFortHealthSet ----
    // FFortGameplayAttributeData inherits FGameplayAttributeData:
    //   base +0x008  float BaseValue
    //   base +0x00C  float CurrentValue
    constexpr uintptr_t HealthSet_Health        = 0x030; // FFortGameplayAttributeData
    constexpr uintptr_t HealthSet_MaxHealth     = 0x058; // FFortGameplayAttributeData
    constexpr uintptr_t HealthSet_Shield        = 0x080; // FFortGameplayAttributeData (max)
    constexpr uintptr_t HealthSet_CurrentShield = 0x0A8; // FFortGameplayAttributeData
    constexpr uintptr_t AttributeData_CurrentValue = 0x00C; // float within FFortGameplayAttributeData

    // ---- USceneComponent ----
    constexpr uintptr_t SceneComp_RelativeLocation = 0x11C; // FVector (284d)
    constexpr uintptr_t SceneComp_RelativeRotation = 0x128; // FRotator (296d)
    constexpr uintptr_t SceneComp_RelativeScale3D  = 0x134; // FVector

    // ---- USkeletalMeshComponent ----
    // Inherits USkinnedMeshComponent → UMeshComponent → UPrimitiveComponent → USceneComponent
    constexpr uintptr_t SkeletalMesh_CachedBoneTransforms = 0x750; // TArray<FTransform> (1872d)

    // ---- UCharacterMovementComponent ----
    constexpr uintptr_t CharMove_GravityScale  = 0x150; // float (336d)
    constexpr uintptr_t CharMove_MaxWalkSpeed  = 0x1AC; // float (428d)
    constexpr uintptr_t CharMove_MaxWalkSpeedCrouched = 0x1B0; // float
}

// ---- Fortnite bone indices (UE4 Mannequin skeleton, Fortnite 18.40) ----
namespace BoneIndex
{
    constexpr int Pelvis       = 0;
    constexpr int Spine01      = 1;
    constexpr int Spine02      = 2;
    constexpr int Spine03      = 3;
    constexpr int Neck         = 65;
    constexpr int Head         = 66;
    constexpr int UpperArmL    = 9;
    constexpr int LowerArmL    = 10;
    constexpr int HandL        = 11;
    constexpr int UpperArmR    = 38;
    constexpr int LowerArmR    = 39;
    constexpr int HandR        = 40;
    constexpr int ThighL       = 71;
    constexpr int CalfL        = 72;
    constexpr int FootL        = 73;
    constexpr int ThighR       = 78;
    constexpr int CalfR        = 79;
    constexpr int FootR        = 80;
}
