#include "stdafx.hpp"

// 전역 변수 (간소화된 예제를 위해)
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
GLFWwindow* g_window = nullptr;

// 헬퍼 함수 선언 (본 코드에서는 생략)
bool CreateDeviceD3D(GLFWwindow* hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();

// **[1] 초기화 함수**
bool InitializeApp(const char* title, int width, int height)
{
    // 1. GLFW 초기화
    if (!glfwInit())
        return false;

    // GLFW에 DX11 사용을 알림 (Windowing만 GLFW가 담당)
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // 2. GLFW 창 생성
    g_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (g_window == nullptr) {
        glfwTerminate();
        return false;
    }

    // 3. DirectX 11 장치 초기화
    if (!CreateDeviceD3D(g_window)) {
        CleanupDeviceD3D();
        glfwTerminate();
        return false;
    }

    // 4. ImGui 컨텍스트 생성
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // 키보드 네비게이션 활성화

    // 5. ImGui 스타일 설정 (선택 사항)
    ImGui::StyleColorsDark();

    // 6. 백엔드 초기화
    // GLFW 바인딩 초기화
    ImGui_ImplGlfw_InitForOther(g_window, true);
    // DirectX 11 바인딩 초기화
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    return true;
}

// **[2] 정리 함수**
void CleanupApp()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    glfwDestroyWindow(g_window);
    glfwTerminate();
}

// **[3] 메인 함수**
int main()
{
    if (!InitializeApp("ImGui Console Test", 1280, 720)) {
        return 1;
    }

    // 렌더링 루프
    while (!glfwWindowShouldClose(g_window))
    {
        glfwPollEvents();

        // A. 새 ImGui 프레임 시작
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // B. **여기에 UI 코드를 작성합니다!**
        {
            // 간단한 콘솔 창 예제 (여기에 사용자 로직 추가)
            ImGui::Begin("Debug Console");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            // 입력 필드 예제 (콘솔 명령 입력 시 필요)
            static char buf[256] = "";
            if (ImGui::InputText("Command", buf, IM_ARRAYSIZE(buf), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                // 엔터키 입력 시 명령어 처리 로직
                // printf("Command Entered: %s\n", buf);
                buf[0] = '\0'; // 입력 필드 초기화
            }
            ImGui::End();
        }

        // C. 렌더링
        ImGui::Render();

        const float clear_color_with_alpha[4] = { 0.45f, 0.55f, 0.60f, 1.00f }; // 배경 색상
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // D. 버퍼 교체 및 화면 표시
        g_pSwapChain->Present(1, 0); // V-Sync 켜기 (1)
    }

    CleanupApp();

    return 0;
}

// **주의:** CreateDeviceD3D, CleanupDeviceD3D, CreateRenderTarget, CleanupRenderTarget, WndProc 등의 DX11 및 GLFW 관련 헬퍼 함수 구현이 필요합니다.


#include <tchar.h>
// 렌더링 타겟을 생성하고 해제하는 함수
void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// DX11 장치를 초기화하는 함수
bool CreateDeviceD3D(GLFWwindow* window)
{
    // GLFW 창 핸들 가져오기 (HWND는 Win32에서 창을 식별하는 핸들입니다.)
    HWND hWnd = glfwGetWin32Window(window);

    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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
    // createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG; // 디버그 모드 사용 시

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

// DX11 장치를 정리하는 함수
void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

