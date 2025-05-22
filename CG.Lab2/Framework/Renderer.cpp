//
// Created by gentletrombone on 22.05.2025.
//

#include "Renderer.h"

Renderer::Renderer() : hWnd(nullptr) {}
Renderer::~Renderer() {}

bool Renderer::Initialize(HWND hWnd, UINT width, UINT height) {
    this->hWnd = hWnd;
    return graphics.Initialize(hWnd, width, height);
}

void Renderer::Update(float deltaTime) {
    totalTime += deltaTime;
    frameCount++;

    if (totalTime >= 1.0f) {
        float fps = frameCount / totalTime;
        std::wstring title = L"My3DApp - FPS: " + std::to_wstring(fps);
        SetWindowText(hWnd, title.c_str());
        totalTime = 0.0f;
        frameCount = 0;
    }
}

void Renderer::Render() {
    graphics.Render();
}
