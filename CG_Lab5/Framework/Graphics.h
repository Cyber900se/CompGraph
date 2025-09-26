//
// Created by gentletrombone on 22.05.2025.
//

#ifndef GRAPHICS_H
#define GRAPHICS_H

#pragma once
#include <windows.h>
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <memory>
#include <d3dcompiler.h>

#include "InputHandler.h"
#include "Cameras/Camera.h"
#include "Cameras/ThirdPersonCamera.h"
#include "Geometry/Scene.h"

struct Light {
    DirectX::XMFLOAT3 position; // 12 байт
    float intensity;            // +4 = 16 байт
    DirectX::XMFLOAT3 color;    // 12 байт
    float padding;              // +4 = 16 байт
}; // = 32 байта, совпадает с HLSL


struct PSConstants {
    DirectX::XMFLOAT3 ambientColor;
    int numLights;

    struct Light {
        DirectX::XMFLOAT3 position;
        float intensity;           // выравнивание 16 байт
        DirectX::XMFLOAT3 color;
        float padding;
    };
    Light lights[100];

    DirectX::XMFLOAT3 cameraPos;
    float padding2;
};

class Graphics {
public:
    Graphics();
    ~Graphics();

    bool Initialize(HWND hWnd, UINT width, UINT height);
    void Render(float totalTime, float width, float height, std::vector<DirectX::XMFLOAT3> lightPositions);
    void Update(float deltaTime, InputHandler& input);
    void Resize(UINT width, UINT height);
    void HandleInput(const InputHandler &input, float deltaTime);

private:
    struct VSConstants {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
        DirectX::XMMATRIX worldInvTranspose;
    };

    enum class ProjectionMode {
        Normal,
        Wide,
        Narrow,
        Ortho
    };

    bool InitDeviceAndSwapChain(HWND hWnd, UINT width, UINT height);
    bool InitShaders(HWND hWnd);
    bool InitCamera();

    void ToggleCamera();
    void UpdateProjection(UINT width, UINT height);

    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
    ID3D11DepthStencilView* depthStencilView;
    ID3D11RenderTargetView* renderTargetView;
    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;
    ID3D11InputLayout* inputLayout;
    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;
    ID3D11Buffer* constantBuffer;
    ID3D11RasterizerState* rastState;
    ID3D11DepthStencilState* depthStencilState;
    ID3D11Buffer* psConstantBuffer;

    std::unique_ptr<ThirdPersonCamera> thirdPersonCamera;
    Camera* activeCamera = nullptr;

    UINT width = 0;
    UINT height = 0;

    DirectX::XMMATRIX projectionMatrix;
    ProjectionMode currentProjection = ProjectionMode::Normal;

    Scene scene;
};

#endif //GRAPHICS_H