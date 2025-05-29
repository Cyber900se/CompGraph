//
// Created by gentletrombone on 22.05.2025.
//

#include "Window.h"

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) {
    switch (umessage)
    {
        case WM_KEYDOWN:
        {
            // If a key is pressed send it to the input object so it can record that state.
            std::cout << "Key: " << static_cast<unsigned int>(wparam) << std::endl;

            if (static_cast<unsigned int>(wparam) == 27) PostQuitMessage(0);
            return 0;
        }
        default:
        {
            return DefWindowProc(hwnd, umessage, wparam, lparam);
        }
    }
}

Window::Window(HINSTANCE hInst, const wchar_t* applicationName, UINT width, UINT height)
    : hInstance(hInst) {

    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = applicationName;

    RegisterClassEx(&wc);

    RECT windowRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    auto dwStyle = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_THICKFRAME;

    auto posX = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    auto posY = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

    hwnd = CreateWindowEx(WS_EX_APPWINDOW, applicationName, applicationName,
        dwStyle,
        posX, posY,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr, nullptr, hInstance, nullptr);

    ShowWindow(hwnd, SW_SHOW);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    ShowCursor(true);
}

Window::~Window() {
    if (hwnd) {
        DestroyWindow(hwnd);
    }
}

bool Window::ProcessMessages() {
    MSG msg = {};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    if (msg.message == WM_QUIT)
        return true;
    return false;
}