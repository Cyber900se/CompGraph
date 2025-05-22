//
// Created by gentletrombone on 22.05.2025.
//
#include "Window.h"
#include <iostream>

bool Window::Initialize(HINSTANCE hInstance, int width, int height, LPCWSTR title) {
    m_hInstance = hInstance;

    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, hInstance, nullptr, nullptr, nullptr, nullptr, title, nullptr };
    RegisterClassEx(&wc);

    RECT wr = { 0, 0, width, height };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    m_hwnd = CreateWindowW(title, title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
        nullptr, nullptr, hInstance, nullptr);

    return m_hwnd != nullptr;
}

void Window::Show() {
    ShowWindow(m_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(m_hwnd);
}

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    if (msg == WM_KEYDOWN && wparam == VK_ESCAPE)
        PostQuitMessage(0);
    return DefWindowProc(hwnd, msg, wparam, lparam);
}