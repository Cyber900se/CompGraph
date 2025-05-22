//
// Created by gentletrombone on 22.05.2025.
//
#include "Window.h"
#include <iostream>

bool Window::Initialize(HINSTANCE hInstance, int width, int height, LPCWSTR title) {
    m_hInstance = hInstance;

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
    wc.lpszClassName = title;

    RegisterClassEx(&wc);

    RECT windowRect = { 0, 0, width, height };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    auto dwStyle = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_THICKFRAME;
    auto posX = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    auto posY = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

    m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, title, title,
        dwStyle,
        posX, posY,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr, nullptr, hInstance, nullptr);

    return m_hwnd != nullptr;
}

void Window::Show() {
    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
    SetFocus(m_hwnd);
    ShowCursor(true);

    //UpdateWindow(m_hwnd);
}

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