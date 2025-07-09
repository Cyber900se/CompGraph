//
// Created by gentletrombone on 22.05.2025.
//

#ifndef APPLICATION_H
#define APPLICATION_H

#pragma once
#include "Window.h"
#include "Graphics.h"
#include "Renderer.h"
#include <chrono>


class Application {
public:
    Application();
    ~Application();
    bool Initialize(HINSTANCE hInstance);
    int Run();

private:
    Window* window = nullptr;
    Renderer renderer;
    UINT width = 900;
    UINT height = 900;
    static inline const wchar_t* applicationName = L"Solar System";

    bool InitWindow(HINSTANCE hInstance);
    void Update(float deltaTime);
    void Render();
    void HandleResize(UINT width, UINT height);
    void HandleInput(const InputHandler& input, float deltaTime);

};

#endif //APPLICATION_H
