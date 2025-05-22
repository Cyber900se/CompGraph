//
// Created by gentletrombone on 22.05.2025.
//

#include "Graphics.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

struct Vertex {
    XMFLOAT4 position;
    XMFLOAT4 color;
};

Graphics::Graphics() {}
Graphics::~Graphics() {}

bool Graphics::Initialize(HWND hWnd, UINT width, UINT height) {
    if (!InitDeviceAndSwapChain(hWnd, width, height)) return false;
    if (!InitShaders()) return false;
    if (!InitGeometry()) return false;
    return true;
}

bool Graphics::InitDeviceAndSwapChain(HWND hWnd, UINT width, UINT height) {
    DXGI_SWAP_CHAIN_DESC swapDesc = {};
    swapDesc.BufferCount = 1;
    swapDesc.BufferDesc.Width = width;
    swapDesc.BufferDesc.Height = height;
    swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.OutputWindow = hWnd;
    swapDesc.SampleDesc.Count = 1;
    swapDesc.Windowed = TRUE;
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featureLevel;
    if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE,
        nullptr, D3D11_CREATE_DEVICE_DEBUG, nullptr, 0,
        D3D11_SDK_VERSION, &swapDesc, &swapChain,
        &device, &featureLevel, &context)))
        return false;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    device->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView);

    CD3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.CullMode = D3D11_CULL_NONE;
    rastDesc.FillMode = D3D11_FILL_SOLID;
    device->CreateRasterizerState(&rastDesc, &rasterState);

    return true;
}

bool Graphics::InitShaders() {
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, psBlob, error;

    HRESULT hr = D3DCompileFromFile(L"Shaders/MyVeryFirstShader.hlsl", nullptr, nullptr,
        "VSMain", "vs_5_0", 0, 0, &vsBlob, &error);
    if (FAILED(hr)) return false;

    hr = D3DCompileFromFile(L"Shaders/MyVeryFirstShader.hlsl", nullptr, nullptr,
        "PSMain", "ps_5_0", 0, 0, &psBlob, &error);
    if (FAILED(hr)) return false;

    device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);
    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(XMFLOAT4), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    device->CreateInputLayout(layoutDesc, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
    return true;
}

bool Graphics::InitGeometry() {
    Vertex vertices[] = {
        { XMFLOAT4(0.5f, 0.5f, 0.0f, 1.0f), XMFLOAT4(1, 0, 0, 1) },
        { XMFLOAT4(-0.5f, -0.5f, 0.0f, 1.0f), XMFLOAT4(0, 0, 1, 1) },
        { XMFLOAT4(0.5f, -0.5f, 0.0f, 1.0f), XMFLOAT4(0, 1, 0, 1) },
        { XMFLOAT4(-0.5f, 0.5f, 0.0f, 1.0f), XMFLOAT4(1, 1, 0, 1) },
    };

    DWORD indices[] = {
        0, 1, 2,
        0, 3, 1
    };

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices;

    device->CreateBuffer(&vbDesc, &vbData, &vertexBuffer);

    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.ByteWidth = sizeof(indices);
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices;

    device->CreateBuffer(&ibDesc, &ibData, &indexBuffer);

    return true;
}

void Graphics::Render() {
    FLOAT clearColor[4] = { 0.1f, 0.1f, 0.3f, 1.0f };
    context->ClearRenderTargetView(renderTargetView.Get(), clearColor);

    context->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), nullptr);
    context->RSSetState(rasterState.Get());

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->IASetInputLayout(inputLayout.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->VSSetShader(vertexShader.Get(), nullptr, 0);
    context->PSSetShader(pixelShader.Get(), nullptr, 0);
    context->DrawIndexed(6, 0, 0);

    swapChain->Present(1, 0);
}