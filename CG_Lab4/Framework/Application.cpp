#include "Application.h"
#include "InputHandler.h"

Application::Application() {}

Application::~Application() {}

bool Application::InitWindow(HINSTANCE hInstance) {
    static const wchar_t* applicationName = L"MyD3D11App";
    window = new Window(hInstance, applicationName, width, height);

    return window->GetHWND() && IsWindow(window->GetHWND());
}

bool Application::Initialize(HINSTANCE hInstance) {
    if (!InitWindow(hInstance)) {
        MessageBox(nullptr, L"Window creation failed", L"Error", MB_OK);
        return false;
    }

    if (!window->GetHWND() || !IsWindow(window->GetHWND())) {
        MessageBox(nullptr, L"Invalid window handle", L"Error", MB_OK);
        return false;
    }

    if (!renderer.Initialize(window->GetHWND(), width, height)) {
        MessageBox(nullptr, L"Renderer initialization failed", L"Error", MB_OK);
        return false;
    }

    return true;
}

int Application::Run() {
    auto prevTime = std::chrono::steady_clock::now();
    while (!window->ProcessMessages()) {
        auto curTime = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(curTime - prevTime).count() / 1000000.0f;
        prevTime = curTime;

        Update(deltaTime);
        Render();
    }
    return 0;
}

void Application::Update(float deltaTime) {
    InputHandler& input = window->GetInputHandler();
    const float paddleSpeed = 1.5f * deltaTime;

    if (input.IsKeyDown('W')) {
        renderer.MoveLeftPaddle(paddleSpeed);
    }
    if (input.IsKeyDown('S')) {
        renderer.MoveLeftPaddle(-paddleSpeed);
    }

    if (input.IsKeyDown(VK_UP)) {
        renderer.MoveRightPaddle(paddleSpeed);
    }
    if (input.IsKeyDown(VK_DOWN)) {
        renderer.MoveRightPaddle(-paddleSpeed);
    }

    if (input.IsKeyDown(VK_ESCAPE)) {
        PostQuitMessage(0);
    }

    renderer.Update(deltaTime);
}

void Application::Render() {
    renderer.Render(width, height);
}
