//
// Created by gentletrombone on 22.05.2025.
//

#include "Graphics.h"
#include <d3dcompiler.h>
#include <array>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

Graphics::Graphics() {}
Graphics::~Graphics() {}

bool Graphics::Initialize(HWND hWnd, UINT width, UINT height) {
    if (!InitDeviceAndSwapChain(hWnd, width, height)) return false;
    if (!InitShaders(hWnd)) return false;
    if (!InitGeometry()) return false;
    return true;
}

bool Graphics::InitDeviceAndSwapChain(HWND hWnd, UINT width, UINT height) {
    DXGI_SWAP_CHAIN_DESC swapDesc = {};

    swapDesc.BufferCount = 2;
    swapDesc.BufferDesc.Width = width;
    swapDesc.BufferDesc.Height = height;
    swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.OutputWindow = hWnd;
    swapDesc.Windowed = true;
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    swapDesc.SampleDesc.Count = 1;
    swapDesc.SampleDesc.Quality = 0;

    D3D_FEATURE_LEVEL featureLevel[] = { D3D_FEATURE_LEVEL_11_1 };

    if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE,
        nullptr, D3D11_CREATE_DEVICE_DEBUG,featureLevel, 1,
        D3D11_SDK_VERSION, &swapDesc, &swapChain,
        &device, nullptr, &context)))
        return false;

        ID3D11Texture2D* backTex;
        swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backTex);
        device->CreateRenderTargetView(backTex, nullptr, &renderTargetView);

        CD3D11_RASTERIZER_DESC rastDesc = {};
        rastDesc.CullMode = D3D11_CULL_NONE;
        rastDesc.FillMode = D3D11_FILL_SOLID;
        device->CreateRasterizerState(&rastDesc, &rastState);

        return true;
}

bool Graphics::InitShaders(HWND hWnd) {
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, psBlob, error;

    HRESULT res = D3DCompileFromFile(L"Shaders/MyVeryFirstShader.hlsl", nullptr, nullptr,
        "VSMain", "vs_5_0", 0, 0, &vsBlob, &error);
    if (FAILED(res)) {
        // If the shader failed to compile it should have written something to the error message.
        if (error) {
            char* compileErrors = (char*)(error->GetBufferPointer());

            std::cout << compileErrors << std::endl;
        }
        // If there was nothing in the error message then it simply could not find the shader file itself.
        else
        {
            MessageBox(hWnd, L"MyVeryFirstShader.hlsl", L"Missing Shader File", MB_OK);
        }
        return false;
    }

    D3D_SHADER_MACRO Shader_Macros[] = { "", "1", "TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)", nullptr, nullptr };

    res = D3DCompileFromFile(L"Shaders/MyVeryFirstShader.hlsl", Shader_Macros, nullptr,
        "PSMain", "ps_5_0", 0, 0, &psBlob, &error);
    if (FAILED(res)) return false;

    device->CreateVertexShader(
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        nullptr, &vertexShader);
    device->CreatePixelShader(
        psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(),
        nullptr, &pixelShader);

    D3D11_INPUT_ELEMENT_DESC inputElements[] = {
        D3D11_INPUT_ELEMENT_DESC {
            "POSITION",
            0,
            DXGI_FORMAT_R32G32B32A32_FLOAT,
            0,
            0,
            D3D11_INPUT_PER_VERTEX_DATA,
            0},
        D3D11_INPUT_ELEMENT_DESC {
            "COLOR",
            0,
            DXGI_FORMAT_R32G32B32A32_FLOAT,
            0,
            D3D11_APPEND_ALIGNED_ELEMENT,
            D3D11_INPUT_PER_VERTEX_DATA,
            0}
    };

    device->CreateInputLayout(
        inputElements,
        2,
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        &inputLayout);
    return true;
}

bool Graphics::InitGeometry() {
    DirectX::XMFLOAT4 points[] = {
        DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
        DirectX::XMFLOAT4(-0.5f, -0.5f, 0.5f, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),
        DirectX::XMFLOAT4(0.5f, -0.5f, 0.5f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),
        DirectX::XMFLOAT4(-0.5f, 0.5f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
    };
    int indices[] = { 0, 1, 2, 0, 3, 1 };

    D3D11_BUFFER_DESC vertexBufDesc = {};
    vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufDesc.CPUAccessFlags = 0;
    vertexBufDesc.MiscFlags = 0;
    vertexBufDesc.StructureByteStride = 0;
    vertexBufDesc.ByteWidth = sizeof(DirectX::XMFLOAT4) * std::size(points);

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = points;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;
    device->CreateBuffer(&vertexBufDesc, &vertexData, &vertexBuffer);

    D3D11_BUFFER_DESC indexBufDesc = {};
    indexBufDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufDesc.CPUAccessFlags = 0;
    indexBufDesc.MiscFlags = 0;
    indexBufDesc.StructureByteStride = 0;
    indexBufDesc.ByteWidth = sizeof(int) * std::size(indices);

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    device->CreateBuffer(&indexBufDesc, &indexData, &indexBuffer);

    return true;
}

void Graphics::Render(float totalTime, float width, float height) {
    context->ClearState();
    context->RSSetState(rastState);

    UINT strides[] = { 32 };
    UINT offsets[] = { 0 };

    D3D11_VIEWPORT viewport = {};
    viewport.Width = width;
    viewport.Height = height;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1.0f;

    context->RSSetViewports(1, &viewport);

    context->IASetInputLayout(inputLayout);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    context->IASetVertexBuffers(0, 1, &vertexBuffer, strides, offsets);
    context->VSSetShader(vertexShader, nullptr, 0);
    context->PSSetShader(pixelShader, nullptr, 0);

    context->OMSetRenderTargets(1, &renderTargetView, nullptr);

    float color[] = { totalTime, 0.1f, 0.1f, 1.0f };
    context->ClearRenderTargetView(renderTargetView, color);

    context->DrawIndexed(6, 0, 0);

    context->OMSetRenderTargets(0, nullptr, nullptr);

    swapChain->Present(1, 0);

}
