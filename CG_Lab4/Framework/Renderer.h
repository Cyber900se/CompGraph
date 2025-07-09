//
// Created by gentletrombone on 22.05.2025.
//

#ifndef RENDERER_H
#define RENDERER_H

#pragma once
#include <windows.h>
#include "Graphics.h"
#include "InputHandler.h"


class Renderer {
public:
    Renderer();
    ~Renderer();

    void HandleInput(const InputHandler& input, float deltaTime);
    bool Initialize(HWND hWnd, UINT width, UINT height);
    bool IsInitialized() const { return initialized; }
    void Update(float deltaTime, const wchar_t* applicationName);
    void Render(float width, float height);
    void Resize(UINT width, UINT height);

private:
    Graphics graphics;
    HWND hWnd;
    float totalTime = 0.0f;
    float timeAccumulator = 0.0f;
    unsigned int frameCount = 0;
    bool initialized = false;
};


#endif //RENDERER_H
