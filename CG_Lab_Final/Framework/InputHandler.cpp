#include "InputHandler.h"

InputHandler::InputHandler()
    : mouseX(0), mouseY(0), prevMouseX(0), prevMouseY(0), mouseWheelDelta(0.0f) {}

void InputHandler::SetKeyDown(Keys key) {
    keyStates[key] = true;
}

void InputHandler::SetKeyUp(Keys key) {
    keyStates[key] = false;
}

bool InputHandler::IsKeyDown(Keys key) const {
    auto it = keyStates.find(key);
    return it != keyStates.end() && it->second;
}

// Новый метод для проверки нажатия в этом кадре
bool InputHandler::WasKeyPressedThisFrame(Keys key) const {
    bool curr = false, prev = false;
    auto itCurr = keyStates.find(key);
    if (itCurr != keyStates.end()) curr = itCurr->second;

    auto itPrev = prevKeyStates.find(key);
    if (itPrev != prevKeyStates.end()) prev = itPrev->second;

    return curr && !prev;
}

// Мышь
void InputHandler::SetMouseButtonDown(Keys button) { SetKeyDown(button); }
void InputHandler::SetMouseButtonUp(Keys button) { SetKeyUp(button); }
bool InputHandler::IsMouseButtonDown(Keys button) const { return IsKeyDown(button); }

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
    return { static_cast<float>(mouseX - prevMouseX),
             static_cast<float>(mouseY - prevMouseY) };
}

// Обновляем состояние клавиш и мыши для нового кадра
void InputHandler::ResetFrameState() {
    prevKeyStates = keyStates;
    prevMouseX = mouseX;
    prevMouseY = mouseY;
    mouseWheelDelta = 0.0f;
}

// Методы для камеры
bool InputHandler::IsMovingForward() const { return IsKeyDown(Keys::W) || IsKeyDown(Keys::Up); }
bool InputHandler::IsMovingBackward() const { return IsKeyDown(Keys::S) || IsKeyDown(Keys::Down); }
bool InputHandler::IsMovingLeft() const { return IsKeyDown(Keys::A) || IsKeyDown(Keys::Left); }
bool InputHandler::IsMovingRight() const { return IsKeyDown(Keys::D) || IsKeyDown(Keys::Right); }
bool InputHandler::IsMovingUp() const { return IsKeyDown(Keys::Space) || IsKeyDown(Keys::E); }
bool InputHandler::IsMovingDown() const { return IsKeyDown(Keys::LeftControl) || IsKeyDown(Keys::Q); }
bool InputHandler::IsCameraRotating() const { return IsMouseButtonDown(Keys::RightButton); }
