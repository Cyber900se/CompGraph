#include "Application.h"


Application::Application() {}

Application::~Application() {}

bool Application::InitWindow(HINSTANCE hInstance) {
    window = new Window(hInstance, applicationName, width, height);
    return window->GetHWND() != nullptr;
}

bool Application::Initialize(HINSTANCE hInstance) {
    if (!InitWindow(hInstance)) {
        MessageBox(nullptr, L"Window creation failed", L"Error", MB_OK);
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

        if (window->CheckSizeChanged()) {
            HandleResize(window->GetWidth(), window->GetHeight());
        }

        InputHandler& input = window->GetInputHandler();

        HandleInput(input, deltaTime);

        Update(deltaTime, input);
        Render();

        input.ResetFrameState();
    }
    return 0;
}

void Application::Update(float deltaTime, InputHandler& input) {
    renderer.HandleInput(window->GetInputHandler(), deltaTime);
    renderer.Update(deltaTime, applicationName, input);
}

void Application::HandleResize(UINT newWidth, UINT newHeight) {
    width = newWidth;
    height = newHeight;

    if (window) {
        window->SetSize(width, height);
    }

    if (renderer.IsInitialized()) {
        renderer.Resize(width, height);
    }
}

void Application::HandleInput(const InputHandler& input, float deltaTime) {
    if (input.IsKeyDown(Keys::Escape)) {
        PostQuitMessage(0);
    }
    renderer.HandleInput(input, deltaTime);
}

void Application::Render() {
    renderer.Render(width, height);
}
