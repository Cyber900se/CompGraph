//
// Created by gentletrombone on 22.05.2025.
//

#include "Graphics.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")


struct VSConstants {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
};

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
        D3D11_SDK_VERSION, &swapDesc, &swapChain, &device, nullptr, &context)))
        return false;

    ID3D11Texture2D* backTex;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backTex);
    device->CreateRenderTargetView(backTex, nullptr, &renderTargetView);

    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthBuffer;
    device->CreateTexture2D(&depthDesc, nullptr, &depthBuffer);
    device->CreateDepthStencilView(depthBuffer.Get(), nullptr, &depthStencilView);

    CD3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.CullMode = D3D11_CULL_NONE;
    rastDesc.FillMode = D3D11_FILL_SOLID;
    device->CreateRasterizerState(&rastDesc, &rastState);

    return true;
}

bool Graphics::InitShaders(HWND hWnd) {
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, psBlob, error;

    HRESULT res = D3DCompileFromFile(L"Shaders/MyVeryFirstShader.hlsl", nullptr, nullptr,
        "VSMain", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vsBlob, &error);
    if (FAILED(res)) {
        // If the shader failed to compile, it should have written something to the error message.
        if (error) {
            char* compileErrors = (char*)(error->GetBufferPointer());

            std::cout << compileErrors << std::endl;
        }
        // If there was nothing in the error message, then it simply could not find the shader file itself.
        else
        {
            MessageBox(hWnd, L"MyVeryFirstShader.hlsl", L"Missing Shader File", MB_OK);
        }
        return false;
    }

    D3D_SHADER_MACRO Shader_Macros[] = { "", "1", "TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)", nullptr, nullptr };

    res = D3DCompileFromFile(L"Shaders/MyVeryFirstShader.hlsl", Shader_Macros, nullptr,
        "PSMain", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &psBlob, &error);
    if (FAILED(res)) {
        // If the shader failed to compile, it should have written something to the error message.
        if (error) {
            char* compileErrors = (char*)(error->GetBufferPointer());

            std::cout << compileErrors << std::endl;
        }
        // If there was nothing in the error message, then it simply could not find the shader file itself.
        else
        {
            MessageBox(hWnd, L"MyVeryFirstShader.hlsl", L"Missing Shader File", MB_OK);
        }
        return false;
    }

    device->CreateVertexShader(
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        nullptr, &vertexShader);

    D3D11_BUFFER_DESC cbd = {};
    cbd.ByteWidth = sizeof(VSConstants);
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    device->CreateBuffer(&cbd, nullptr, &constantBuffer);

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

/*bool Graphics::InitGeometry() {
    DirectX::XMFLOAT4 points[] = {
        DirectX::XMFLOAT4(0.25f, 0.25f, 0.5f, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
        DirectX::XMFLOAT4(-0.25f, -0.25f, 0.5f, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),
        DirectX::XMFLOAT4(0.25f, -0.25f, 0.5f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),
        DirectX::XMFLOAT4(-0.25f, 0.25f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
    };
    int indices[] = { 0, 1, 2, 0, 3, 1 };

    D3D11_BUFFER_DESC vertexBufDesc = {};
    vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufDesc.ByteWidth = sizeof(DirectX::XMFLOAT4) * std::size(points);
    vertexBufDesc.CPUAccessFlags = 0;
    vertexBufDesc.MiscFlags = 0;
    vertexBufDesc.StructureByteStride = 0;

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
    context->ClearState();

    return true;
}*/

bool Graphics::InitGeometry() {
    const int numSegments = 64;
    const float radius = 0.25f;

    struct Vertex {
        DirectX::XMFLOAT4 pos;
        DirectX::XMFLOAT4 col;
    };

    std::vector<Vertex> vertices;
    std::vector<UINT> indices;

    // Центр круга
    vertices.push_back({
        DirectX::XMFLOAT4(0.0f, 0.0f, 0.5f, 1.0f),
        DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)
    });

    // Вершины по окружности
    for (int i = 0; i <= numSegments; ++i) {
        float angle = DirectX::XM_2PI * i / numSegments;
        float x = cosf(angle) * radius;
        float y = sinf(angle) * radius;

        vertices.push_back({
            DirectX::XMFLOAT4(x, y, 0.5f, 1.0f),
            DirectX::XMFLOAT4(
                0.5f + 0.5f * cosf(angle),
                0.5f + 0.5f * sinf(angle),
                1.0f,
                1.0f)
        });
    }

    // Индексы — "треугольный фан"
    for (UINT i = 1; i <= numSegments; ++i) {
        indices.push_back(0);        // центр
        indices.push_back(i);
        indices.push_back(i + 1 <= numSegments ? i + 1 : 1); // wrap
    }

    // Создание vertex buffer
    D3D11_BUFFER_DESC vertexBufDesc = {};
    vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufDesc.ByteWidth = UINT(vertices.size() * sizeof(Vertex));
    vertexBufDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = vertices.data();

    HRESULT hr = device->CreateBuffer(&vertexBufDesc, &vertexData, &vertexBuffer);
    if (FAILED(hr)) return false;

    // Создание index buffer
    D3D11_BUFFER_DESC indexBufDesc = {};
    indexBufDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufDesc.ByteWidth = UINT(indices.size() * sizeof(UINT));
    indexBufDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices.data();

    hr = device->CreateBuffer(&indexBufDesc, &indexData, &indexBuffer);
    if (FAILED(hr)) return false;

    // Устанавливаем количество индексов (если нужно в будущем)
    indexCount = UINT(indices.size());

    context->ClearState();
    return true;
}


void Graphics::Render(float totalTime, float width, float height) {
    context->RSSetState(rastState);

    //float x = -1.0f + totalTime * 0.3f;
    float y = sinf(totalTime * 2.0f) * 0.3f;
    //DirectX::XMMATRIX world = DirectX::XMMatrixTranslation(x, y, 0.0f);

    DirectX::XMVECTOR eyePos = DirectX::XMVectorSet(0.0f, 0.0f, -2.0f, 0.0f);     // Камера немного позади объекта по Z
    DirectX::XMVECTOR focusPos = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);   // Смотрит на центр
    DirectX::XMVECTOR upDir = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);       // Вверх — по Y

    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(eyePos, focusPos, upDir);

    DirectX::XMMATRIX proj = DirectX::XMMatrixOrthographicLH(2.0f, 2.0f, 0.1f, 10.0f);

    /*VSConstants vsConst = {
        world,
        view,
        proj
    };

    context->UpdateSubresource(constantBuffer, 0, nullptr, &vsConst, 0, 0);*/

    D3D11_VIEWPORT viewport = {};
    viewport.Width = width;
    viewport.Height = height;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1.0f;
    context->RSSetViewports(1, &viewport);

    UINT strides[] = { 32 };
    UINT offsets[] = { 0 };

    context->IASetInputLayout(inputLayout);
    context->IASetVertexBuffers(0, 1, &vertexBuffer, strides, offsets);
    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->VSSetShader(vertexShader, nullptr, 0);
    context->VSSetConstantBuffers(0, 1, &constantBuffer);
    context->PSSetShader(pixelShader, nullptr, 0);

    float color[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    context->ClearRenderTargetView(renderTargetView, color);
    context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

    struct InstanceData {
        float offsetX;
        float offsetY;
        float amplitude;
        float speed;
    };

    std::vector<InstanceData> instances = {
        { -1.0f,  0.5f, 0.1f, 1.0f },
        { -0.3f,  0.3f, 0.2f, 1.5f },
        {  0.4f,  0.0f, 0.3f, 2.0f },
        {  1.0f, -0.4f, 0.15f, 2.5f }
    };

    for (const auto& inst : instances) {
        float x = -1.0f + totalTime * 0.3f;
        float y = inst.offsetY + sinf(totalTime * inst.speed) * inst.amplitude;

        DirectX::XMMATRIX world = DirectX::XMMatrixTranslation(x, y, 0.0f);

        VSConstants vsConst = {
            world,
            view,
            proj
        };

        context->UpdateSubresource(constantBuffer, 0, nullptr, &vsConst, 0, 0);
        //context->DrawIndexed(6, 0, 0);
        context->DrawIndexed(indexCount, 0, 0);

    }

    swapChain->Present(1, 0);
}
