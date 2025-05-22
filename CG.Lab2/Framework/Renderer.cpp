//
// Created by gentletrombone on 22.05.2025.
//

#include "Renderer.h"

Renderer::Renderer(Graphics* gfx)
    : m_gfx(gfx) {}

void Renderer::RenderFrame() {
    // Очистка экрана
    m_gfx->Clear(0.1f, 0.1f, 0.1f, 1.0f);

    // Рисуем примитивы (двойной треугольник = прямоугольник)
    m_gfx->DrawIndexed(6);

    // Отображаем результат
    m_gfx->Present();
}
