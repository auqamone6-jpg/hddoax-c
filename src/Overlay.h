#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <functional>
#include <string>

class Overlay
{
public:
    using RenderCallback = std::function<void(float /*w*/, float /*h*/)>;

    // Create the transparent overlay window and initialize D3D11 + ImGui.
    bool Init(const wchar_t* targetWindowName);

    // Run the main loop. Calls `renderCb` every frame until the target window disappears
    // or the user closes the overlay.
    void RunLoop(RenderCallback renderCb);

    // Shut down cleanly.
    void Shutdown();

    float ScreenW() const { return m_screenW; }
    float ScreenH() const { return m_screenH; }

    static Overlay& Get() { static Overlay inst; return inst; }

private:
    Overlay() = default;
    ~Overlay() = default;

    bool CreateOverlayWindow(const wchar_t* targetTitle);
    bool InitD3D11();
    void InitImGui();
    void Resize(int w, int h);
    void ReleaseRenderTarget();
    void CreateRenderTarget();

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    HWND  m_hwnd    = nullptr;
    HWND  m_target  = nullptr;

    ID3D11Device*           m_device         = nullptr;
    ID3D11DeviceContext*    m_context         = nullptr;
    IDXGISwapChain*         m_swapChain       = nullptr;
    ID3D11RenderTargetView* m_renderTargetView= nullptr;

    float m_screenW = 1920.f;
    float m_screenH = 1080.f;
    bool  m_running = false;
};
