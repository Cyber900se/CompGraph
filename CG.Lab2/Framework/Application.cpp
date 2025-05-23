//
// Created by gentletrombone on 22.05.2025.
//

#include "Application.h"

Application::Application() {

}

Application::~Application() {

}

bool Application::InitWindow(HINSTANCE hInstance) {
    window = new Window(hInstance, L"MyD3D11App", width, height);
    return true;
}

bool Application::Initialize(HINSTANCE hInstance) {
    if (!InitWindow(hInstance)) return false;
    if (!renderer.Initialize(window->GetHWND(), width, height)) return false;
    return true;
}

int Application::Run() {
    auto prevTime = std::chrono::steady_clock::now();
    while (!window->ProcessMessages()) {
        auto curTime = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(curTime - prevTime).count() / 1000000.0f;
        prevTime = curTime;
        Render();
        Update(deltaTime);
    }
    return 0;
}

void Application::Update(float deltaTime) {
    renderer.Update(deltaTime);
}

void Application::Render() {
    renderer.Render(width, height);
}
