#ifndef WINDOW_H
#define WINDOW_H

#pragma once
#include <windows.h>
#include <iostream>
#include "InputHandler.h"

class Window {
public:
    Window(HINSTANCE hInstance, const wchar_t* title, UINT width, UINT height);
    ~Window();
    void SetSize(UINT width, UINT height);
    bool CheckSizeChanged();
    HWND GetHWND() const { return hwnd; }
    bool ProcessMessages();

    UINT GetWidth() const { return width; }
    UINT GetHeight() const { return height; }
    InputHandler& GetInputHandler() { return inputHandler; }
    const InputHandler& GetInputHandler() const { return inputHandler; }

private:
    UINT width;
    UINT height;
    HWND hwnd;
    HINSTANCE hInstance;
    bool sizeChanged = false;
    InputHandler inputHandler;
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    LRESULT HandleMessage(UINT umessage, WPARAM wparam, LPARAM lparam);
};


#endif //WINDOW_H
