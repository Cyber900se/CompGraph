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
        totalTime -= 1.0f;

        WCHAR text[256];
        swprintf_s(text, ARRAYSIZE(text), L"FPS: %f", fps);

        SetWindowTextW(hWnd, text);

        frameCount = 0;
    }
}

void Renderer::Render(float width, float height) {
    graphics.Render(totalTime, width, height);
}
