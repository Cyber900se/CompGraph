//
// Created by gentletrombone on 22.05.2025.
//

#include "Application.h"
#include <stdexcept>

Application::Application(HINSTANCE hInstance)
    : hInstance(hInstance), hwnd(nullptr), graphics(nullptr), renderer(nullptr) {}

Application::~Application() {
    delete renderer;
    delete graphics;
}

bool Application::Initialize() {
    if (!InitWindow()) return false;

    graphics = new Graphics(hwnd, width, height);
    if (!graphics->Initialize()) return false;

    renderer = new Renderer(graphics);
    return true;
}

void Application::Run() {
    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            Render();  // вызываем отрисовку каждый кадр
        }
    }
}


bool Application::InitWindow() {
    WNDCLASS wc = {};
    wc.lpfnWndProc = Application::WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = windowClassName;

    RegisterClass(&wc);

    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    hwnd = CreateWindowEx(
        0, windowClassName, L"My DirectX App",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwnd) return false;

    ShowWindow(hwnd, SW_SHOW);
    return true;
}

LRESULT CALLBACK Application::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}


