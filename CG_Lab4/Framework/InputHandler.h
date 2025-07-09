#pragma once
#include <array>
#include <DirectXMath.h>

#include "Keys.h"

class InputHandler {
public:
    InputHandler();

    // Управление состоянием клавиш
    void SetKeyDown(Keys key);
    void SetKeyUp(Keys key);
    bool IsKeyDown(Keys key) const;

    // Управление состоянием мыши
    void SetMouseButtonDown(Keys button);
    void SetMouseButtonUp(Keys button);
    void SetMousePosition(int x, int y);
    void AddMouseWheelDelta(int delta);

    // Получение состояния
    bool IsMouseButtonDown(Keys button) const;
    float GetMouseWheelDelta() const;
    DirectX::XMFLOAT2 GetMouseDelta() const;
    void ResetFrameState();

    // Управление камерой
    bool IsMovingForward() const;
    bool IsMovingBackward() const;
    bool IsMovingLeft() const;
    bool IsMovingRight() const;
    bool IsMovingUp() const;
    bool IsMovingDown() const;
    bool IsCameraRotating() const;

private:
    static constexpr size_t KEYS_COUNT = 600;
    std::array<bool, KEYS_COUNT> keyStates;

    int mouseX = 0;
    int mouseY = 0;
    int prevMouseX = 0;
    int prevMouseY = 0;
    float mouseWheelDelta = 0.0f;
};