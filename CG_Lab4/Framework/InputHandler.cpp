#include "InputHandler.h"
#include <DirectXMath.h>

InputHandler::InputHandler() {
    keyStates.fill(false);
}

void InputHandler::SetKeyDown(Keys key) {
    const int index = static_cast<int>(key);
    if (index >= 0 && index < KEYS_COUNT) {
        keyStates[index] = true;
    }
}

void InputHandler::SetKeyUp(Keys key) {
    const int index = static_cast<int>(key);
    if (index >= 0 && index < KEYS_COUNT) {
        keyStates[index] = false;
    }
}

bool InputHandler::IsKeyDown(Keys key) const {
    const int index = static_cast<int>(key);
    return (index >= 0 && index < KEYS_COUNT) && keyStates[index];
}

void InputHandler::SetMouseButtonDown(Keys button) {
    SetKeyDown(button);
}

void InputHandler::SetMouseButtonUp(Keys button) {
    SetKeyUp(button);
}

bool InputHandler::IsMouseButtonDown(Keys button) const {
    return IsKeyDown(button);
}

void InputHandler::SetMousePosition(int x, int y) {
    mouseX = x;
    mouseY = y;
}

void InputHandler::AddMouseWheelDelta(int delta) {
    mouseWheelDelta += static_cast<float>(delta) / 120.0f;
}

float InputHandler::GetMouseWheelDelta() const {
    return mouseWheelDelta;
}

DirectX::XMFLOAT2 InputHandler::GetMouseDelta() const {
    return {
        static_cast<float>(mouseX - prevMouseX),
        static_cast<float>(mouseY - prevMouseY)
    };
}

void InputHandler::ResetFrameState() {
    prevMouseX = mouseX;
    prevMouseY = mouseY;
    mouseWheelDelta = 0.0f;
}

// Camera control methods
bool InputHandler::IsMovingForward() const {
    return IsKeyDown(Keys::W) || IsKeyDown(Keys::Up);
}

bool InputHandler::IsMovingBackward() const {
    return IsKeyDown(Keys::S) || IsKeyDown(Keys::Down);
}

bool InputHandler::IsMovingLeft() const {
    return IsKeyDown(Keys::A) || IsKeyDown(Keys::Left);
}

bool InputHandler::IsMovingRight() const {
    return IsKeyDown(Keys::D) || IsKeyDown(Keys::Right);
}

bool InputHandler::IsMovingUp() const {
    return IsKeyDown(Keys::Space) || IsKeyDown(Keys::E);
}

bool InputHandler::IsMovingDown() const {
    return IsKeyDown(Keys::LeftControl) || IsKeyDown(Keys::Q);
}

bool InputHandler::IsCameraRotating() const {
    return IsMouseButtonDown(Keys::RightButton);
}