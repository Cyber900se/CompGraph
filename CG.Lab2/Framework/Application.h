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

    bool InitWindow(HINSTANCE hInstance);
    void Update(float deltaTime);
    void Render();

    int width = 800;
    int height = 800;
};

#endif //APPLICATION_H
