//
// Created by gentletrombone on 22.05.2025.
//

#ifndef APPLICATION_H
#define APPLICATION_H

#pragma once
#include <windows.h>
#include "Graphics.h"
#include "Renderer.h"

class Application {
public:
    Application(HINSTANCE hInstance);
    ~Application();

    bool Initialize();
    void Run();

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    bool InitWindow();

private:
    HINSTANCE hInstance;
    HWND hwnd;
    Graphics* graphics;
    Renderer* renderer;

    const wchar_t* windowClassName = L"MyAppWindow";
    int width = 800;
    int height = 600;
};

#endif //APPLICATION_H
