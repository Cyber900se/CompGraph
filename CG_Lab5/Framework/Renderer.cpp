#include "Renderer.h"

Renderer::Renderer() : hWnd(nullptr), initialized(false) {}
Renderer::~Renderer() {}

bool Renderer::Initialize(HWND hWnd, UINT width, UINT height) {
    this->hWnd = hWnd;
    initialized = graphics.Initialize(hWnd, width, height);
    return initialized;
}

void Renderer::Update(float deltaTime, const wchar_t* applicationName, InputHandler& input) {
    totalTime += deltaTime;
    timeAccumulator += deltaTime;
    frameCount++;

    if (timeAccumulator >= 1.0f) {
        float fps = frameCount / timeAccumulator;
        timeAccumulator -= 1.0f;

        WCHAR text[256];
        swprintf_s(text, ARRAYSIZE(text), L"%s FPS: %.1f", applicationName, fps);

        SetWindowTextW(hWnd, text);

        frameCount = 0;
    }
    graphics.Update(deltaTime, input);
}

void Renderer::Render(float width, float height) {
    graphics.Render(totalTime, width, height);
}

void Renderer::Resize(UINT width, UINT height) {
    graphics.Resize(width, height);
}

void Renderer::HandleInput(const InputHandler& input, float deltaTime) {
    graphics.HandleInput(input, deltaTime);
}