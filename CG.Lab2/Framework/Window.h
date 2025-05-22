//
// Created by gentletrombone on 22.05.2025.
//

#ifndef WINDOW_H
#define WINDOW_H

#pragma once
#include <windows.h>

class Window {
public:
    bool Initialize(HINSTANCE hInstance, int width, int height, LPCWSTR title);
    void Show();
    HWND GetHWND() const { return m_hwnd; }

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
    HWND m_hwnd;
    HINSTANCE m_hInstance;
};

#endif //WINDOW_H
