//
// Created by gentletrombone on 22.05.2025.
//

#include "Application.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_KEYDOWN && wParam == VK_ESCAPE) {
        PostQuitMessage(0);
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

Application::Application() : hWnd(nullptr) {}
Application::~Application() {}

bool Application::InitWindow(HINSTANCE hInstance) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WndProc,
                      0, 0, hInstance, LoadIcon(nullptr, IDI_WINLOGO), LoadCursor(nullptr, IDC_ARROW),
                      (HBRUSH)GetStockObject(BLACK_BRUSH), nullptr, applicationName, LoadIcon(nullptr, IDI_WINLOGO) };

    RegisterClassEx(&wc);

    RECT rect = { 0, 0, (LONG)screenWidth, (LONG)screenHeight };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    hWnd = CreateWindow(applicationName, applicationName,
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) return false;

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);
    return true;
}

bool Application::Initialize(HINSTANCE hInstance) {
    if (!InitWindow(hInstance)) return false;
    if (!renderer.Initialize(hWnd, screenWidth, screenHeight)) return false;
    return true;
}

int Application::Run() {
    MSG msg = {};
    auto prevTime = std::chrono::steady_clock::now();

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            auto currentTime = std::chrono::steady_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - prevTime).count();
            prevTime = currentTime;

            Update(deltaTime);
            Render();
        }
    }

    return static_cast<int>(msg.wParam);
}

void Application::Update(float deltaTime) {
    renderer.Update(deltaTime);
}

void Application::Render() {
    renderer.Render(screenWidth, screenHeight);
}
