#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "Memory.h"
#include "GameReader.h"
#include "ESP.h"
#include "Aimbot.h"
#include "Menu.h"
#include "Overlay.h"
#include "Settings.h"

static const wchar_t* FORTNITE_EXE    = L"FortniteClient-Win64-Shipping.exe";
static const wchar_t* FORTNITE_WINDOW = L"Fortnite  ";

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    // --- Attach to Fortnite process ---
    if (!Memory::Open(FORTNITE_EXE))
    {
        MessageBoxW(nullptr,
            L"Could not open FortniteClient-Win64-Shipping.exe.\n"
            L"Make sure Fortnite is running and this tool is run as Administrator.",
            L"Fortnite Utility", MB_ICONERROR | MB_OK);
        return 1;
    }

    // --- Create transparent overlay ---
    Overlay& overlay = Overlay::Get();
    if (!overlay.Init(FORTNITE_WINDOW))
    {
        MessageBoxW(nullptr,
            L"Failed to create overlay window / D3D11 device.",
            L"Fortnite Utility", MB_ICONERROR | MB_OK);
        Memory::Close();
        return 1;
    }

    // --- Per-frame state ---
    GameReader reader;

    // --- Main loop ---
    overlay.RunLoop([&](float w, float h)
    {
        reader.Update();
        const GameReader::GameState& state = reader.State();

        ESP::Render(state, w, h);
        Aimbot::Update(state, w, h);
        Menu::Render();
    });

    overlay.Shutdown();
    Memory::Close();
    return 0;
}
