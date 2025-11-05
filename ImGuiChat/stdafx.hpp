#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX // Windows.h와 다른 라이브러리(GLFW) 간의 min/max 매크로 충돌 방지

#include <Windows.h>

// DirectX 11 헤더
#include <d3d11.h> 

// ----------------------------------------------------
// GLFW 및 Native Win32 연동을 위한 추가 (HWND 획득)
// ----------------------------------------------------
#define GLFW_EXPOSE_NATIVE_WIN32 // Win32 API 노출 요청
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>    // glfwGetWin32Window 함수 정의 포함

// ImGui 핵심
#include <imgui.h>
// ImGui 백엔드
// --- 제거: <imgui_impl_win32.h> ---
#include <imgui_impl_dx11.h>     // DirectX 11 렌더링
#include <imgui_impl_glfw.h>     // GLFW 입력 및 창 관리 (Win32 대신 GLFW 사용)

// 기타 유틸리티
#include <tchar.h>
#include <vector>
#include <string>
// ... 프로젝트에서 사용하는 다른 헤더 ...