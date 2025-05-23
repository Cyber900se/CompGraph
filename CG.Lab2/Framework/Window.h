//
// Created by gentletrombone on 22.05.2025.
//

#ifndef WINDOW_H
#define WINDOW_H

#pragma once
#include <windows.h>
#include <iostream>

class Window {
public:
    Window(HINSTANCE hInstance, const wchar_t* title, UINT width, UINT height);
    ~Window();

    HWND GetHWND() const { return hwnd; }
    bool ProcessMessages();

private:
    HWND hwnd;
    HINSTANCE hInstance;
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};


#endif //WINDOW_H
