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
#include "Cameras/FPSCamera.h"
#include "Cameras/OrbitCamera.h"
#include "Geometry/Scene.h"

class Graphics {
public:
    Graphics();
    ~Graphics();

    bool Initialize(HWND hWnd, UINT width, UINT height);
    void Render(float totalTime, float width, float height);
    void Update(float deltaTime, InputHandler& input);
    void Resize(UINT width, UINT height);
    void HandleInput(const InputHandler &input, float deltaTime);

private:
    struct VSConstants {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
    };

    enum class ProjectionMode {
        Normal,
        Wide,
        Narrow,
        Ortho
    };

    struct RenderObject;

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

    std::unique_ptr<OrbitCamera> orbitCamera;
    std::unique_ptr<FPSCamera> fpsCamera;
    Camera* activeCamera = nullptr;

    UINT width = 0;
    UINT height = 0;

    DirectX::XMMATRIX projectionMatrix;
    ProjectionMode currentProjection = ProjectionMode::Normal;

    Scene scene;
};

#endif //GRAPHICS_H