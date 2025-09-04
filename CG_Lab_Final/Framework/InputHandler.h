#pragma once
#include "Keys.h"
#include <DirectXMath.h>
#include <unordered_map>

class InputHandler {
public:
    InputHandler();

    // Клавиши
    void SetKeyDown(Keys key);
    void SetKeyUp(Keys key);
    bool IsKeyDown(Keys key) const;
    bool WasKeyPressedThisFrame(Keys key) const;

    // Мышь
    void SetMouseButtonDown(Keys button);
    void SetMouseButtonUp(Keys button);
    bool IsMouseButtonDown(Keys button) const;

    void SetMousePosition(int x, int y);
    void AddMouseWheelDelta(int delta);
    float GetMouseWheelDelta() const;
    DirectX::XMFLOAT2 GetMouseDelta() const;

    void ResetFrameState();

    // Методы для камеры
    bool IsMovingForward() const;
    bool IsMovingBackward() const;
    bool IsMovingLeft() const;
    bool IsMovingRight() const;
    bool IsMovingUp() const;
    bool IsMovingDown() const;
    bool IsCameraRotating() const;

private:
    std::unordered_map<Keys, bool> keyStates;      // текущее состояние клавиш
    std::unordered_map<Keys, bool> prevKeyStates;  // состояние клавиш на предыдущем кадре
    int mouseX, mouseY;
    int prevMouseX, prevMouseY;
    float mouseWheelDelta;
};
