#include "Application.h"

#include <random>



std::vector<DirectX::XMFLOAT3> InitLights()
{
    constexpr int MAX_LIGHTS = 100; // например, 10 источников

    std::vector<DirectX::XMFLOAT3> lightPositions;

    std::mt19937 rng(42); // фиксированный seed, чтобы "рандом" был стабильный
    std::uniform_real_distribution<float> dist(-5.0f, 5.0f);

    lightPositions.resize(MAX_LIGHTS);
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        lightPositions[i] = DirectX::XMFLOAT3(dist(rng), 2.0f, dist(rng));
    }
    return lightPositions;
}

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
    std::vector<DirectX::XMFLOAT3> lightPositions = InitLights();
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
        Render(lightPositions);

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

void Application::Render(std::vector<DirectX::XMFLOAT3> lightPositions) {
    renderer.Render(width, height, lightPositions);
}
