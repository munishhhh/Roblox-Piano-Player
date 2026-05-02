#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include <dwmapi.h>
#include <shlobj.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "ui.h"
#include "midi_player.h"

// Data
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// For acrylic/blur effect
enum ACCENT_STATE {
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
    ACCENT_INVALID_STATE = 5
};

struct ACCENT_POLICY {
    ACCENT_STATE AccentState;
    DWORD AccentFlags;
    DWORD GradientColor;
    DWORD AnimationId;
};

struct WINDOWCOMPOSITIONATTRIBDATA {
    DWORD Attrib;
    PVOID pvData;
    SIZE_T cbData;
};

typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);

void EnableBlur(HWND hwnd) {
    HMODULE hUser = GetModuleHandle(TEXT("user32.dll"));
    if (hUser) {
        pSetWindowCompositionAttribute setWindowCompositionAttribute = (pSetWindowCompositionAttribute)GetProcAddress(hUser, "SetWindowCompositionAttribute");
        if (setWindowCompositionAttribute) {
            ACCENT_POLICY policy = { ACCENT_ENABLE_ACRYLICBLURBEHIND, 2, 0x01000000, 0 }; // 0x01000000 for slightly dark transparent
            WINDOWCOMPOSITIONATTRIBDATA data = { 19, &policy, sizeof(ACCENT_POLICY) }; // WCA_ACCENT_POLICY = 19
            setWindowCompositionAttribute(hwnd, &data);
        }
    }
    
    // Also set DWM Blur as fallback
    DWM_BLURBEHIND bb = {0};
    bb.dwFlags = DWM_BB_ENABLE;
    bb.fEnable = true;
    bb.hRgnBlur = NULL;
    DwmEnableBlurBehindWindow(hwnd, &bb);
}

// Load texture function for DX11
bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height)
{
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL) return false;

    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = image_width;
    desc.Height = image_height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    ID3D11Texture2D* pTexture = NULL;
    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = image_data;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;
    g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
    pTexture->Release();

    *out_width = image_width;
    *out_height = image_height;
    stbi_image_free(image_data);

    return true;
}

void RegisterContextMenu() {
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);

    auto RegisterExt = [&](const char* ext) {
        std::string keyPath = std::string("Software\\Classes\\SystemFileAssociations\\") + ext + "\\shell\\PlayWithRobloxMidi";
        HKEY hKey;
        if (RegCreateKeyExA(HKEY_CURRENT_USER, keyPath.c_str(), 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
            RegSetValueExA(hKey, "", 0, REG_SZ, (const BYTE*)"Play with Brothers Regedit Roblox Midi Player", 46);
            std::string iconStr = std::string("\"") + exePath + "\"";
            RegSetValueExA(hKey, "Icon", 0, REG_SZ, (const BYTE*)iconStr.c_str(), iconStr.length() + 1);
            
            HKEY hCommandKey;
            if (RegCreateKeyExA(hKey, "command", 0, NULL, 0, KEY_WRITE, NULL, &hCommandKey, NULL) == ERROR_SUCCESS) {
                std::string command = std::string("\"") + exePath + "\" \"%1\"";
                RegSetValueExA(hCommandKey, "", 0, REG_SZ, (const BYTE*)command.c_str(), command.length() + 1);
                RegCloseKey(hCommandKey);
            }
            RegCloseKey(hKey);
        }
    };

    RegisterExt(".mid");
    RegisterExt(".midi");
    
    // Notify Windows Explorer that file associations have changed
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
}

// Global MidiPlayer
MidiPlayer g_MidiPlayer;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    RegisterContextMenu();

    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv && argc > 1) {
        char mbPath[MAX_PATH];
        WideCharToMultiByte(CP_UTF8, 0, argv[1], -1, mbPath, MAX_PATH, NULL, NULL);
        g_MidiPlayer.AddToLibrary(mbPath);
        g_MidiPlayer.AddToQueue(mbPath);
        g_MidiPlayer.LoadFromQueue(0);
    }
    if (argv) LocalFree(argv);

    // Create application window
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("RobloxMidiClass"), NULL };
    ::RegisterClassEx(&wc);
    
    // WS_POPUP for frameless window
    HWND hwnd = ::CreateWindowEx(WS_EX_LAYERED, wc.lpszClassName, _T("Brothers Regedit Roblox Midi Player"), WS_POPUP, 100, 100, 900, 600, NULL, NULL, wc.hInstance, NULL);

    // Set transparency color key (using black as transparent for DWM/DX11 to blend)
    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
    
    EnableBlur(hwnd);
    DragAcceptFiles(hwnd, TRUE);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Load AI icons
    int w, h;
    ID3D11ShaderResourceView* srv = nullptr;
    if(LoadTextureFromFile("play.png", &srv, &w, &h)) UI::texPlay = (ImTextureID)srv;
    if(LoadTextureFromFile("pause.png", &srv, &w, &h)) UI::texPause = (ImTextureID)srv;
    if(LoadTextureFromFile("stop.png", &srv, &w, &h)) UI::texStop = (ImTextureID)srv;
    if(LoadTextureFromFile("close.png", &srv, &w, &h)) UI::texClose = (ImTextureID)srv;
    if(LoadTextureFromFile("minimize.png", &srv, &w, &h)) UI::texMinimize = (ImTextureID)srv;
    if(LoadTextureFromFile("maximize.png", &srv, &w, &h)) UI::texMaximize = (ImTextureID)srv;
    if(LoadTextureFromFile("logo.png", &srv, &w, &h)) UI::texLogo = (ImTextureID)srv;
    if(LoadTextureFromFile("star_filled.png", &srv, &w, &h)) UI::texStarFilled = (ImTextureID)srv;
    if(LoadTextureFromFile("star_empty.png", &srv, &w, &h)) UI::texStarEmpty = (ImTextureID)srv;
    if(LoadTextureFromFile("prev.png", &srv, &w, &h)) UI::texPrev = (ImTextureID)srv;
    if(LoadTextureFromFile("next.png", &srv, &w, &h)) UI::texNext = (ImTextureID)srv;
    if(LoadTextureFromFile("tab_dash.png", &srv, &w, &h)) UI::texTabDash = (ImTextureID)srv;
    if(LoadTextureFromFile("tab_midi.png", &srv, &w, &h)) UI::texTabMidi = (ImTextureID)srv;
    if(LoadTextureFromFile("tab_settings.png", &srv, &w, &h)) UI::texTabSettings = (ImTextureID)srv;
    if(LoadTextureFromFile("loop.png", &srv, &w, &h)) UI::texLoop = (ImTextureID)srv;
    if(LoadTextureFromFile("discord.png", &srv, &w, &h)) UI::texDiscord = (ImTextureID)srv;
    if(LoadTextureFromFile("github.png", &srv, &w, &h)) UI::texGithub = (ImTextureID)srv;
    if(LoadTextureFromFile("instagram.png", &srv, &w, &h)) UI::texInstagram = (ImTextureID)srv;

    // Scale UI fonts and elements slightly to make them easier to read
    io.FontGlobalScale = 1.15f;

    UI::Init();

    // Main loop
    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (!io.WantCaptureKeyboard) {
            g_MidiPlayer.UpdateGlobalHotkeys();
        }

        UI::Render(g_MidiPlayer);

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // Transparent clear color
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // Changed for transparency
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_NCHITTEST:
    {
        LRESULT hit = DefWindowProc(hWnd, msg, wParam, lParam);
        if (hit == HTCLIENT) {
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(hWnd, &pt);
            // Protect window control buttons area (top right 120px) from being draggable
            RECT rect;
            GetClientRect(hWnd, &rect);
            if (pt.y < 40 && pt.x < rect.right - 130)
                return HTCAPTION;
        }
        return hit;
    }
    case WM_DROPFILES:
    {
        HDROP hDrop = (HDROP)wParam;
        UINT count = DragQueryFileA(hDrop, 0xFFFFFFFF, NULL, 0);
        for(UINT i=0; i<count; ++i) {
            char filename[MAX_PATH];
            if (DragQueryFileA(hDrop, i, filename, MAX_PATH)) {
                g_MidiPlayer.AddToLibrary(filename);
                g_MidiPlayer.AddToQueue(filename);
            }
        }
        DragFinish(hDrop);
        return 0;
    }
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
