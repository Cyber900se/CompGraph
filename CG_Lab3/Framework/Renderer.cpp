#include "Renderer.h"

Renderer::Renderer() : hWnd(nullptr) {}
Renderer::~Renderer() {}

bool Renderer::Initialize(HWND hWnd, UINT width, UINT height) {
    this->hWnd = hWnd;
    return graphics.Initialize(hWnd, width, height);
}

void Renderer::Update(float deltaTime) {
    totalTime += deltaTime;
    timeAccumulator += deltaTime;
    frameCount++;

    if (timeAccumulator >= 1.0f) {
        float fps = frameCount / timeAccumulator;
        timeAccumulator -= 1.0f;

        WCHAR text[256];
        swprintf_s(text, ARRAYSIZE(text),
                  L"Pong | Left: %d - Right: %d | FPS: %.1f",
                  graphics.GetLeftPlayerScore(),
                  graphics.GetRightPlayerScore(),
                  fps);

        SetWindowTextW(hWnd, text);

        frameCount = 0;
    }
    graphics.Update(deltaTime);
}

void Renderer::Render(float width, float height) {
    graphics.Render(totalTime, width, height);
}

void Renderer::MoveLeftPaddle(float dy) {
    graphics.MoveLeftPaddle(dy);
}

void Renderer::MoveRightPaddle(float dy) {
    graphics.MoveRightPaddle(dy);
}