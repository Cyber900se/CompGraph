//
// Created by gentletrombone on 22.05.2025.
//

#ifndef RENDERER_H
#define RENDERER_H
#pragma once
#include "Graphics.h"

class Renderer {
public:
    Renderer(Graphics* gfx);
    void RenderFrame();

private:
    Graphics* m_gfx;
};


#endif //RENDERER_H
