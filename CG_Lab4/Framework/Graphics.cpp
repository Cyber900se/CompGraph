#include "Graphics.h"
#include <fstream>

#include "InputHandler.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

struct Vertex {
    DirectX::XMFLOAT4 pos;
    DirectX::XMFLOAT4 col;
};

struct Graphics::RenderObject {
    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;
    UINT indexCount;
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT2 velocity;
    DirectX::XMFLOAT3 rotationAxis;
    float rotationAngle;
};

struct VSConstants {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
};

Graphics::Graphics() {}
Graphics::~Graphics() {}

bool Graphics::Initialize(HWND hWnd, UINT width, UINT height) {
    this->width = width;
    this->height = height;
    return InitDeviceAndSwapChain(hWnd, width, height) &&
           InitShaders(hWnd) &&
           InitGeometry() &&
           InitCamera();
}

void Graphics::Resize(UINT newWidth, UINT newHeight) {
    if (newWidth == 0 || newHeight == 0) return;

    width = newWidth;
    height = newHeight;

    if (renderTargetView) {
        renderTargetView->Release();
        renderTargetView = nullptr;
    }
    if (depthStencilView) {
        depthStencilView->Release();
        depthStencilView = nullptr;
    }

    swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

    ID3D11Texture2D* backBuffer;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&backBuffer));
    if (backBuffer) {
        device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
    }

    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* depthBuffer;
    if (FAILED(device->CreateTexture2D(&depthDesc, nullptr, &depthBuffer))) {
        MessageBox(nullptr, L"Depth buffer creation failed", L"Error", MB_OK);
        return;
    }
    if (depthBuffer) {
        device->CreateDepthStencilView(depthBuffer, nullptr, &depthStencilView);
        depthBuffer->Release();
    }
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
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE,
        nullptr, D3D11_CREATE_DEVICE_DEBUG,featureLevel, 1,
        D3D11_SDK_VERSION, &swapDesc, &swapChain, &device, nullptr, &context);
    if (FAILED(hr)) {
        char msg[64];
        sprintf_s(msg, "D3D11CreateDeviceAndSwapChain failed: 0x%X", hr);
        MessageBoxA(hWnd, msg, "Error", MB_OK);
        return false;
    }

    ID3D11Texture2D* backTex;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&backTex));
    hr = device->CreateRenderTargetView(backTex, nullptr, &renderTargetView);
    backTex->Release();
    if (FAILED(hr)) {
        MessageBox(hWnd, L"CreateRenderTargetView failed", L"Error", MB_OK);
        return false;
    }

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
    hr = device->CreateDepthStencilView(depthBuffer.Get(), nullptr, &depthStencilView);
    if (FAILED(hr)) {
        MessageBox(hWnd, L"CreateDepthStencilView failed", L"Error", MB_OK);
        return false;
    }

    CD3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.CullMode = D3D11_CULL_NONE;
    rastDesc.FillMode = D3D11_FILL_SOLID;
    device->CreateRasterizerState(&rastDesc, &rastState);

    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.ByteWidth = sizeof(VSConstants);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = 0;

    hr = device->CreateBuffer(&cbd, nullptr, &constantBuffer);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create constant buffer!", L"Error", MB_OK);
        return false;
    }

    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
    device->CreateDepthStencilState(&dsDesc, &depthStencilState);

    return true;
}

bool Graphics::InitShaders(HWND hWnd) {
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, psBlob, error;

    HRESULT hr = D3DCompileFromFile(L"Shaders/MyVeryFirstShader.hlsl", nullptr, nullptr,
        "VSMain", "vs_5_0", 0, 0, &vsBlob, &error);
    if (FAILED(hr)) {
        if (error) {
            OutputDebugStringA(static_cast<char *>(error->GetBufferPointer())); // отладочный вывод
            std::ofstream fout("shader-error.txt");
            fout << static_cast<char *>(error->GetBufferPointer());
            fout.close();
        }
        MessageBox(hWnd, L"Shader compilation failed. Check shader-error.txt", L"Error", MB_OK);
        return false;
    }
    if (FAILED(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader))){
        MessageBox(hWnd, L"Vertex Shader creation failed", L"Error", MB_OK);
        return false;
    }

    D3DCompileFromFile(L"Shaders/MyVeryFirstShader.hlsl", nullptr, nullptr,
        "PSMain", "ps_5_0", 0, 0, &psBlob, &error);
    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);

    D3D11_INPUT_ELEMENT_DESC objLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    device->CreateInputLayout(objLayout, ARRAYSIZE(objLayout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);

    vsBlob->Release();
    psBlob->Release();
    if (error) error->Release();

    return true;
}

bool Graphics::InitCamera() {
    fpsCamera.SetPosition(0.0f, 0.0f, -5.0f);
    orbitCamera.SetPosition(0.0f, 0.0f, 0.0f);
    currentCamera = &orbitCamera;
    return true;
}

void Graphics::ToggleCamera() {
    useOrbitCamera = !useOrbitCamera;
    currentCamera = useOrbitCamera ?
        static_cast<Camera*>(&orbitCamera) :
        static_cast<Camera*>(&fpsCamera);
}

void Graphics::HandleInput(const InputHandler& input, float deltaTime) {
    const float moveSpeed = 5.0f * deltaTime;
    const float rotationSpeed = 2.0f * deltaTime;
    const float zoomSpeed = 3.0f * deltaTime;

    if (input.IsKeyDown(Keys::F1)) {
        ToggleCamera();
    }

    if (input.IsMovingForward()) currentCamera->MoveForward(moveSpeed);
    if (input.IsMovingBackward()) currentCamera->MoveBackward(moveSpeed);
    if (input.IsMovingRight()) currentCamera->MoveRight(moveSpeed);
    if (input.IsMovingLeft()) currentCamera->MoveLeft(moveSpeed);
    if (input.IsMovingUp()) currentCamera->MoveUp(moveSpeed);
    if (input.IsMovingDown()) currentCamera->MoveDown(moveSpeed);

    if (input.IsCameraRotating()) {
        auto delta = input.GetMouseDelta();
        currentCamera->Rotate(delta.x * rotationSpeed, delta.y * rotationSpeed);
    }

    float wheelDelta = input.GetMouseWheelDelta();
    if (wheelDelta != 0.0f) {
        currentCamera->Zoom(wheelDelta * zoomSpeed);
    }
}

Graphics::RenderObject Graphics::CreateCubeMesh(float size, const DirectX::XMFLOAT4& color) {
    const float half = size / 2.0f;

    Vertex vertices[] = {
        { {-half, -half, -half, 1.0f}, color },
        { {-half,  half, -half, 1.0f}, color },
        { { half,  half, -half, 1.0f}, color },
        { { half, -half, -half, 1.0f}, color },

        { {-half, -half,  half, 1.0f}, color },
        { {-half,  half,  half, 1.0f}, color },
        { { half,  half,  half, 1.0f}, color },
        { { half, -half,  half, 1.0f}, color }
    };

    UINT indices[] = {
        0,1,2, 0,2,3,   // Передняя грань
        4,6,5, 4,7,6,   // Задняя грань
        4,5,1, 4,1,0,   // Левая грань
        3,2,6, 3,6,7,   // Правая грань
        1,5,6, 1,6,2,   // Верхняя грань
        4,0,3, 4,3,7    // Нижняя грань
    };

    // Создание вершинного буфера
    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = sizeof(vertices);
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vinitData = {};
    vinitData.pSysMem = vertices;

    ID3D11Buffer* vertexBuffer;
    HRESULT hr = device->CreateBuffer(&vbd, &vinitData, &vertexBuffer);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Vertex buffer creation failed", L"Error", MB_OK);
        return { nullptr, nullptr, 0 };
    }


    // Создание индексного буфера
    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = sizeof(indices);
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA iinitData = {};
    iinitData.pSysMem = indices;

    ID3D11Buffer* indexBuffer;
    hr = device->CreateBuffer(&ibd, &iinitData, &indexBuffer);
    if (FAILED(hr)) {
        vertexBuffer->Release();
        MessageBox(nullptr, L"Index buffer creation failed", L"Error", MB_OK);
        return { nullptr, nullptr, 0 };
    }

    return {
        vertexBuffer,
        indexBuffer,
        36,              // Количество индексов
        {0.0f, 0.0f, 0.0f},    // Позиция
        {0.0f, 0.0f},    // Скорость
        {0.0f, 1.0f, 0.0f}, // Ось Y по умолчанию
        0.0f             // Начальный угол
    };
}

bool Graphics::InitGeometry() {

    RenderObject cube = CreateCubeMesh(0.3f, {1.0f, 0.5f, 0.0f, 1.0f});
    cube.position = {0.0f, 0.0f, 0.0f}; // Центр экрана
    objects.push_back(cube);

    return true;
}

void Graphics::Update(float deltaTime) {
    for (auto& obj : objects) {
        obj.rotationAngle += deltaTime * 0.5f;
    }
}

void Graphics::Render(float totalTime, float width, float height) {

    float color[] = { 0.1f, 0.1f, 0.1f, 1.0f };

    if (renderTargetView) {
        context->ClearRenderTargetView(renderTargetView, color);
    }
    if (depthStencilView) {
        context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    }

    context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
    context->OMSetDepthStencilState(depthStencilState, 1); // Включаем тест глубины
    context->RSSetState(rastState);

    context->IASetInputLayout(inputLayout);
    context->VSSetShader(vertexShader, nullptr, 0);
    context->PSSetShader(pixelShader, nullptr, 0);
    context->VSSetConstantBuffers(0, 1, &constantBuffer);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    D3D11_VIEWPORT viewport = {};
    viewport.Width = width;
    viewport.Height = height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    context->RSSetViewports(1, &viewport);

    float aspectRatio = (height > 0) ? (width / height) : 1.0f;
    DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XM_PIDIV4,
        aspectRatio,
        0.1f,
        100.0f
    );

    for (RenderObject& obj : objects) {

        DirectX::XMMATRIX world = DirectX::XMMatrixRotationAxis(
            XMLoadFloat3(&obj.rotationAxis),
            obj.rotationAngle
        ) * DirectX::XMMatrixTranslation(obj.position.x, obj.position.y, 0.0f);

        VSConstants vsConst = {
            XMMatrixTranspose(world),
            XMMatrixTranspose(currentCamera->GetViewMatrix()),
            XMMatrixTranspose(proj)
        };

        if (constantBuffer) {
            context->UpdateSubresource(constantBuffer, 0, nullptr, &vsConst, 0, 0);
        }

        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        context->IASetVertexBuffers(0, 1, &obj.vertexBuffer, &stride, &offset);
        context->IASetIndexBuffer(obj.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        context->DrawIndexed(obj.indexCount, 0, 0);
    }
    swapChain->Present(1, 0);
}
