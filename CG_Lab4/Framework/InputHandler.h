#ifndef INPUT_H
#define INPUT_H

#pragma once
#include <Windows.h>
#include <array>

class InputHandler {
public:
    InputHandler();
    ~InputHandler();
    void SetKeyDown(UINT key);
    void SetKeyUp(UINT key);
    bool IsKeyDown(UINT key) const;

private:
    std::array<bool, 256> keys{};
};

#endif //INPUT_H
