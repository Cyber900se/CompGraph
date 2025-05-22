//
// Created by gentletrombone on 22.05.2025.
//

#include "Graphics.h"
#include <d3dcompiler.h>
#include <chrono>
#include <iostream>

using namespace Microsoft::WRL;
using namespace DirectX;

Graphics::Graphics(HWND hwnd, UINT width, UINT height)
    : m_hwnd(hwnd), m_width(width), m_height(height) {}

bool Graphics::Initialize() {
    DXGI_SWAP_CHAIN_DESC swapDesc = {};
    swapDesc.BufferCount = 2;
    swapDesc.BufferDesc.Width = m_width;
    swapDesc.BufferDesc.Height = m_height;
    swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.OutputWindow = m_hwnd;
    swapDesc.Windowed = true;
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapDesc.SampleDesc.Count = 1;

    D3D_FEATURE_LEVEL featureLevel;
    auto result = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_DEBUG,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &swapDesc,
        &m_swapChain,
        &m_device,
        &featureLevel,
        &m_context);

    if (FAILED(result)) return false;

    // Получаем backbuffer и создаём RTV
    ComPtr<ID3D11Texture2D> backBuffer;
    result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer);
    if (FAILED(result)) return false;

    result = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_rtv);
    if (FAILED(result)) return false;

    // Настройка растеризатора
    D3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.CullMode = D3D11_CULL_NONE;

    result = m_device->CreateRasterizerState(&rastDesc, &m_rasterState);
    if (FAILED(result)) return false;

    m_context->RSSetState(m_rasterState.Get());

    return CreateTriangle();  // создаём вершины и шейдеры
}

bool Graphics::CreateTriangle() {
    struct Vertex {
        XMFLOAT4 pos;
        XMFLOAT4 color;
    };

    Vertex vertices[] = {
        {{  0.5f,  0.5f, 0.5f, 1.0f }, { 1, 0, 0, 1 }},
        {{ -0.5f, -0.5f, 0.5f, 1.0f }, { 0, 0, 1, 1 }},
        {{  0.5f, -0.5f, 0.5f, 1.0f }, { 0, 1, 0, 1 }},
        {{ -0.5f,  0.5f, 0.5f, 1.0f }, { 1, 1, 1, 1 }},
    };

    DWORD indices[] = { 0, 1, 2, 1, 0, 3 };

    // Vertex Buffer
    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = sizeof(vertices);
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;

    HRESULT hr = m_device->CreateBuffer(&vbd, &initData, &m_vb);
    if (FAILED(hr)) return false;

    // Index Buffer
    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = sizeof(indices);
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA iinitData = {};
    iinitData.pSysMem = indices;

    hr = m_device->CreateBuffer(&ibd, &iinitData, &m_ib);
    if (FAILED(hr)) return false;

    return LoadShaders(L"./Shaders/MyVeryFirstShader.hlsl");
}

bool Graphics::LoadShaders(const std::wstring& shaderFile) {
    ComPtr<ID3DBlob> vsBlob, psBlob, errorBlob;

    // Vertex Shader
    HRESULT hr = D3DCompileFromFile(shaderFile.c_str(), nullptr, nullptr, "VSMain", "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vsBlob, &errorBlob);

    if (FAILED(hr)) {
        if (errorBlob) std::cout << (char*)errorBlob->GetBufferPointer() << std::endl;
        return false;
    }

    hr = m_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vs);
    if (FAILED(hr)) return false;

    // Pixel Shader
    D3D_SHADER_MACRO macros[] = { {"TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)"}, {nullptr, nullptr} };

    hr = D3DCompileFromFile(shaderFile.c_str(), macros, nullptr, "PSMain", "ps_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &psBlob, &errorBlob);

    if (FAILED(hr)) {
        if (errorBlob) std::cout << (char*)errorBlob->GetBufferPointer() << std::endl;
        return false;
    }

    hr = m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_ps);
    if (FAILED(hr)) return false;

    // Input layout
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    hr = m_device->CreateInputLayout(layoutDesc, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_layout);
    return SUCCEEDED(hr);
}

void Graphics::Clear(float r, float g, float b, float a) {
    float clearColor[] = { r, g, b, a };
    m_context->ClearRenderTargetView(m_rtv.Get(), clearColor);

    D3D11_VIEWPORT vp = {};
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = static_cast<float>(m_width);
    vp.Height = static_cast<float>(m_height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;

    m_context->RSSetViewports(1, &vp);
    m_context->OMSetRenderTargets(1, m_rtv.GetAddressOf(), nullptr);
}

void Graphics::DrawIndexed(UINT indexCount) {
    UINT stride = sizeof(XMFLOAT4) * 2;
    UINT offset = 0;

    m_context->IASetInputLayout(m_layout.Get());
    m_context->IASetVertexBuffers(0, 1, m_vb.GetAddressOf(), &stride, &offset);
    m_context->IASetIndexBuffer(m_ib.Get(), DXGI_FORMAT_R32_UINT, 0);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_context->VSSetShader(m_vs.Get(), nullptr, 0);
    m_context->PSSetShader(m_ps.Get(), nullptr, 0);

    m_context->DrawIndexed(indexCount, 0, 0);
}

void Graphics::Present() {
    m_swapChain->Present(1, 0);
}

ID3D11DeviceContext* Graphics::GetContext() const {
    return m_context.Get();
}

ID3D11Buffer* Graphics::GetVertexBuffer() const {
    return m_vb.Get();
}

ID3D11Buffer* Graphics::GetIndexBuffer() const {
    return m_ib.Get();
}
