//
// Created by gentletrombone on 22.05.2025.
//

#ifndef APPLICATION_H
#define APPLICATION_H

#pragma once
#include <windows.h>
#include <chrono>
#include "Renderer.h"

class Application {
public:
    Application();
    ~Application();

    bool Initialize(HINSTANCE hInstance);
    int Run();

private:
    bool InitWindow(HINSTANCE hInstance);
    void Update(float deltaTime);
    void Render();

    HWND hWnd;
    LPCWSTR applicationName = L"My3DApp";
    unsigned int screenWidth = 800;
    unsigned int screenHeight = 800;
    Renderer renderer;
};

#endif //APPLICATION_H
