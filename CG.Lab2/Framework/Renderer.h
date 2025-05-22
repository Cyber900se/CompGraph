//
// Created by gentletrombone on 22.05.2025.
//

#ifndef RENDERER_H
#define RENDERER_H

#pragma once
#include <windows.h>
#include <string>
#include "Graphics.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool Initialize(HWND hWnd, UINT width, UINT height);
    void Update(float deltaTime);
    void Render();

private:
    Graphics graphics;
    HWND hWnd;
    float totalTime = 0.0f;
    unsigned int frameCount = 0;
};


#endif //RENDERER_H
