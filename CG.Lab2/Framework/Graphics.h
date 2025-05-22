//
// Created by gentletrombone on 22.05.2025.
//

#ifndef GRAPHICS_H
#define GRAPHICS_H

#pragma once
#include <windows.h>
#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <string>

class Graphics {
public:
    Graphics(HWND hwnd, UINT width, UINT height);
    bool Initialize();

    void Clear(float r, float g, float b, float a);
    void DrawIndexed(UINT indexCount);
    void Present();

    ID3D11DeviceContext* GetContext() const;
    ID3D11Buffer* GetVertexBuffer() const;
    ID3D11Buffer* GetIndexBuffer() const;

private:
    bool CreateTriangle();
    bool LoadShaders(const std::wstring& shaderFile);

    HWND m_hwnd;
    UINT m_width, m_height;

    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vs;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_ps;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_layout;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterState;

    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vb;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_ib;
};

#endif //GRAPHICS_H