#include "InputHandler.h"

InputHandler::InputHandler() {
    keys.fill(false);
}

InputHandler::~InputHandler() {}

void InputHandler::SetKeyDown(UINT key) {
    if (key < 256) keys[key] = true;
}

void InputHandler::SetKeyUp(UINT key) {
    if (key < 256) keys[key] = false;
}

bool InputHandler::IsKeyDown(UINT key) const {
    return key < 256 && keys[key];
}