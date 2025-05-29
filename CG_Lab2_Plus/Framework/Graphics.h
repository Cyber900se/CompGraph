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

class Graphics {
public:
    Graphics();
    ~Graphics();

    bool Initialize(HWND hWnd, UINT width, UINT height);
    void Render(float totalTime, float width, float height);

private:
    bool InitDeviceAndSwapChain(HWND hWnd, UINT width, UINT height);
    bool InitShaders(HWND hWnd);
    bool InitGeometry();

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
};


#endif //GRAPHICS_H