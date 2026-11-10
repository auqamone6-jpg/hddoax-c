#pragma once
#include "SDK.h"
#include <vector>

class GameReader
{
public:
    static constexpr int MaxPlayers = 100;

    struct GameState
    {
        bool             bValid       = false;
        FMinimalViewInfo CameraPOV{};
        uint8_t          LocalTeam    = 255;
        uintptr_t        LocalPawn    = 0;
        std::vector<PlayerInfo> Players;
    };

    GameReader() = default;

    // Call once per frame. Populates the internal state from the game process.
    bool Update();

    const GameState& State() const { return m_state; }

    // Reset state to empty
    void Reset() { m_state = {}; }

private:
    GameState  m_state;
    uintptr_t  m_worldCache    = 0;
    uintptr_t  m_gameStateCache= 0;

    bool ReadGlobals();
    bool ReadCamera(uintptr_t localPC);
    bool ReadPlayers(uintptr_t gameState);
    bool ReadSinglePlayer(uintptr_t playerStateAddr, PlayerInfo& out);
    bool ReadBones(const PlayerInfo& p, PlayerInfo& out);
    float ReadAttributeCurrentValue(uintptr_t attributeDataAddr) const;
};
