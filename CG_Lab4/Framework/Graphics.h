//
// Created by gentletrombone on 22.05.2025.
//

#ifndef GRAPHICS_H
#define GRAPHICS_H

#pragma once
#include <windows.h>
#include <d3d11.h>
#include <wrl.h>
#include <d3dcompiler.h>
#include <array>
#include <iostream>
#include <DirectXMath.h>
#include <cmath>
#include <vector>

class Graphics {
public:
    Graphics();
    ~Graphics();

    bool Initialize(HWND hWnd, UINT width, UINT height);
    void Render(float totalTime, float width, float height);
    void MoveLeftPaddle(float dy);
    void MoveRightPaddle(float dy);
    void Update(float deltaTime);
    int GetLeftPlayerScore() const { return leftPlayerScore; }
    int GetRightPlayerScore() const { return rightPlayerScore; }
private:
    struct RenderObject;
    bool InitDeviceAndSwapChain(HWND hWnd, UINT width, UINT height);
    bool InitShaders(HWND hWnd);
    RenderObject CreateRectangleMesh(float meshWidth, float meshHeight, DirectX::XMFLOAT4 color);
    RenderObject CreateCircleMesh(float meshSegments, float meshRadius);
    bool InitGeometry();
    void ResetBall();
    void CreateBackground();
    int leftPlayerScore = 0;
    int rightPlayerScore = 0;

    // Background shaders
    ID3D11VertexShader* backgroundVS = nullptr;
    ID3D11PixelShader* backgroundPS = nullptr;
    ID3D11InputLayout* backgroundLayout = nullptr;
    ID3D11Buffer* backgroundVB;
    ID3D11Buffer* backgroundIB;
    int backgroundVertexCount = 0;
    // Object shaders
    ID3D11VertexShader* objectVS = nullptr;
    ID3D11PixelShader* objectPS = nullptr;
    ID3D11InputLayout* objectLayout = nullptr;


    std::vector<RenderObject> objects;
    UINT indexCount = 0;
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
    ID3D11DepthStencilState* depthStencilState = nullptr;
};


#endif //GRAPHICS_H