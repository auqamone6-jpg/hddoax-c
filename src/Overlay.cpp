#include "Overlay.h"
#include "Settings.h"
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <dwmapi.h>
#include <stdexcept>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

static Overlay* g_overlayPtr = nullptr;

LRESULT CALLBACK Overlay::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (g_settings.menu_open)
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wp, lp)) return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_overlayPtr && g_overlayPtr->m_swapChain && wp != SIZE_MINIMIZED)
            g_overlayPtr->Resize(static_cast<int>(LOWORD(lp)), static_cast<int>(HIWORD(lp)));
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

bool Overlay::Init(const wchar_t* targetWindowName)
{
    g_overlayPtr = this;

    if (!CreateOverlayWindow(targetWindowName)) return false;
    if (!InitD3D11()) return false;
    InitImGui();
    return true;
}

bool Overlay::CreateOverlayWindow(const wchar_t* targetTitle)
{
    m_target = FindWindowW(nullptr, targetTitle);
    if (!m_target)
    {
        // Try partial match - Fortnite window title can vary
        // Fall back to screen dimensions
        m_screenW = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
        m_screenH = static_cast<float>(GetSystemMetrics(SM_CYSCREEN));
    }
    else
    {
        RECT rc{};
        GetWindowRect(m_target, &rc);
        m_screenW = static_cast<float>(rc.right  - rc.left);
        m_screenH = static_cast<float>(rc.bottom - rc.top);
    }

    WNDCLASSEXW wc{ sizeof(wc) };
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"FNOverlay";
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassExW(&wc);

    m_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE,
        L"FNOverlay", L"",
        WS_POPUP,
        0, 0,
        static_cast<int>(m_screenW),
        static_cast<int>(m_screenH),
        nullptr, nullptr, wc.hInstance, nullptr
    );

    if (!m_hwnd) return false;

    // Transparent background (alpha = 0 → pass-through)
    SetLayeredWindowAttributes(m_hwnd, 0, 0, LWA_ALPHA);

    // Enable DWM blur / composition so clear color is truly transparent
    DWM_BLURBEHIND bb{};
    bb.dwFlags   = DWM_BB_ENABLE | DWM_BB_BLURREGION;
    bb.hRgnBlur  = CreateRectRgn(0, 0, 1, 1);
    bb.fEnable   = TRUE;
    DwmEnableBlurBehindWindow(m_hwnd, &bb);

    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
    return true;
}

bool Overlay::InitD3D11()
{
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount        = 2;
    sd.BufferDesc.Width   = static_cast<UINT>(m_screenW);
    sd.BufferDesc.Height  = static_cast<UINT>(m_screenH);
    sd.BufferDesc.Format  = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow       = m_hwnd;
    sd.SampleDesc.Count   = 1;
    sd.Windowed           = TRUE;
    sd.SwapEffect         = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    UINT flags = 0;
#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
        &featureLevel, 1, D3D11_SDK_VERSION,
        &sd, &m_swapChain, &m_device, nullptr, &m_context
    );
    if (FAILED(hr)) return false;

    CreateRenderTarget();
    return true;
}

void Overlay::CreateRenderTarget()
{
    ID3D11Texture2D* back = nullptr;
    m_swapChain->GetBuffer(0, IID_PPV_ARGS(&back));
    if (back)
    {
        m_device->CreateRenderTargetView(back, nullptr, &m_renderTargetView);
        back->Release();
    }
}

void Overlay::ReleaseRenderTarget()
{
    if (m_renderTargetView) { m_renderTargetView->Release(); m_renderTargetView = nullptr; }
}

void Overlay::Resize(int w, int h)
{
    ReleaseRenderTarget();
    m_swapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
    m_screenW = static_cast<float>(w);
    m_screenH = static_cast<float>(h);
    CreateRenderTarget();
}

void Overlay::InitImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr; // don't write imgui.ini

    // Style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding  = 6.f;
    style.FrameRounding   = 4.f;
    style.GrabRounding    = 4.f;
    style.WindowBorderSize= 1.f;
    style.FramePadding    = { 8, 4 };

    ImGui_ImplWin32_Init(m_hwnd);
    ImGui_ImplDX11_Init(m_device, m_context);
}

void Overlay::RunLoop(RenderCallback renderCb)
{
    m_running = true;

    while (m_running)
    {
        // ---- Process Windows messages ----
        MSG msg{};
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            if (msg.message == WM_QUIT) { m_running = false; break; }
        }
        if (!m_running) break;

        // ---- Toggle menu on INSERT ----
        static bool insertHeld = false;
        bool insertNow = (GetAsyncKeyState(VK_INSERT) & 0x8000) != 0;
        if (insertNow && !insertHeld)
        {
            g_settings.menu_open = !g_settings.menu_open;

            // Allow or block mouse input on the overlay
            LONG_PTR exStyle = GetWindowLongPtrW(m_hwnd, GWL_EXSTYLE);
            if (g_settings.menu_open)
                exStyle &= ~WS_EX_TRANSPARENT;
            else
                exStyle |= WS_EX_TRANSPARENT;
            SetWindowLongPtrW(m_hwnd, GWL_EXSTYLE, exStyle);

            // Bring overlay to front when menu opens
            if (g_settings.menu_open)
                SetForegroundWindow(m_hwnd);
        }
        insertHeld = insertNow;

        // ---- Sync overlay position with target window ----
        if (m_target && IsWindow(m_target))
        {
            RECT rc{};
            GetWindowRect(m_target, &rc);
            int tw = rc.right  - rc.left;
            int th = rc.bottom - rc.top;
            SetWindowPos(m_hwnd, HWND_TOPMOST, rc.left, rc.top, tw, th, SWP_NOACTIVATE);
            if (tw != static_cast<int>(m_screenW) || th != static_cast<int>(m_screenH))
                Resize(tw, th);
        }

        // ---- Begin frame ----
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // ---- User render callback (ESP + menu) ----
        renderCb(m_screenW, m_screenH);

        // ---- Render + present ----
        ImGui::Render();
        const float clearColor[4] = { 0.f, 0.f, 0.f, 0.f };
        m_context->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
        m_context->ClearRenderTargetView(m_renderTargetView, clearColor);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        m_swapChain->Present(1, 0);

        // ---- Frame-rate cap ----
        if (g_settings.overlay_fps > 0.f)
        {
            DWORD sleepMs = static_cast<DWORD>(1000.f / g_settings.overlay_fps);
            if (sleepMs > 1) Sleep(sleepMs - 1);
        }
    }
}

void Overlay::Shutdown()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    ReleaseRenderTarget();
    if (m_swapChain)  { m_swapChain->Release();  m_swapChain  = nullptr; }
    if (m_context)    { m_context->Release();    m_context    = nullptr; }
    if (m_device)     { m_device->Release();     m_device     = nullptr; }
    if (m_hwnd)       { DestroyWindow(m_hwnd);   m_hwnd       = nullptr; }
    UnregisterClassW(L"FNOverlay", GetModuleHandleW(nullptr));
}
