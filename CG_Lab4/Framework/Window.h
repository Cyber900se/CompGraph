#ifndef WINDOW_H
#define WINDOW_H

#pragma once
#include <windows.h>
#include <iostream>
#include "InputHandler.h"

class Window {
public:
    Window(HINSTANCE hInstance, static const wchar_t* title, UINT width, UINT height);
    ~Window();

    HWND GetHWND() const { return hwnd; }
    bool ProcessMessages();

    InputHandler& GetInputHandler() { return inputHandler; }
    const InputHandler& GetInputHandler() const { return inputHandler; }

private:
    HWND hwnd;
    HINSTANCE hInstance;
    InputHandler inputHandler;
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    LRESULT HandleMessage(UINT umessage, WPARAM wparam, LPARAM lparam);
};


#endif //WINDOW_H
