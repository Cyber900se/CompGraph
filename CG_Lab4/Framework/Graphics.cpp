#include "Graphics.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

Graphics::Graphics() {}
Graphics::~Graphics() {}

bool Graphics::Initialize(HWND hWnd, UINT width, UINT height) {
    this->width = width;
    this->height = height;

    if (!InitDeviceAndSwapChain(hWnd, width, height)) return false;
    if (!InitShaders(hWnd)) return false;
    if (!InitCamera()) return false;

    scene.Initialize(device.Get());
    UpdateProjection(width, height);
    return true;
}

void Graphics::Resize(UINT newWidth, UINT newHeight) {
    if (newWidth == 0 || newHeight == 0) return;
    width = newWidth;
    height = newHeight;

    if (renderTargetView) { renderTargetView->Release(); renderTargetView = nullptr; }
    if (depthStencilView) { depthStencilView->Release(); depthStencilView = nullptr; }

    swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

    ID3D11Texture2D* backBuffer = nullptr;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
    if (backBuffer) {
        device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
        backBuffer->Release();
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

    ID3D11Texture2D* depthBuffer = nullptr;
    if (SUCCEEDED(device->CreateTexture2D(&depthDesc, nullptr, &depthBuffer))) {
        device->CreateDepthStencilView(depthBuffer, nullptr, &depthStencilView);
        depthBuffer->Release();
    }
    UpdateProjection(width, height);
}

bool Graphics::InitDeviceAndSwapChain(HWND hWnd, UINT width, UINT height) {
    DXGI_SWAP_CHAIN_DESC swapDesc = {};
    swapDesc.BufferCount = 2;
    swapDesc.BufferDesc.Width = width;
    swapDesc.BufferDesc.Height = height;
    swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.OutputWindow = hWnd;
    swapDesc.Windowed = true;
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapDesc.SampleDesc.Count = 1;

    D3D_FEATURE_LEVEL featureLevel[] = { D3D_FEATURE_LEVEL_11_1 };
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE,
                                              nullptr, D3D11_CREATE_DEVICE_DEBUG,
                                              featureLevel, 1, D3D11_SDK_VERSION,
                                              &swapDesc, &swapChain, &device, nullptr, &context);
    if (FAILED(hr)) return false;

    ID3D11Texture2D* backTex = nullptr;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backTex));
    hr = device->CreateRenderTargetView(backTex, nullptr, &renderTargetView);
    backTex->Release();
    if (FAILED(hr)) return false;

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

    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.ByteWidth = sizeof(VSConstants);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    device->CreateBuffer(&cbd, nullptr, &constantBuffer);

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
            MessageBox(nullptr, L"Shader Error!", L"Error", MB_OK);
        }
        return false;
    }

    hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
                                    nullptr, &vertexShader);
    if (FAILED(hr)) return false;

    hr = D3DCompileFromFile(L"Shaders/MyVeryFirstShader.hlsl", nullptr, nullptr,
                            "PSMain", "ps_5_0", 0, 0, &psBlob, &error);
    if (FAILED(hr)) return false;

    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);

    D3D11_INPUT_ELEMENT_DESC objLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    device->CreateInputLayout(objLayout, ARRAYSIZE(objLayout), vsBlob->GetBufferPointer(),
                              vsBlob->GetBufferSize(), &inputLayout);

    return true;
}

bool Graphics::InitCamera() {
    orbitCamera = std::make_unique<OrbitCamera>();
    fpsCamera = std::make_unique<FPSCamera>();

    fpsCamera->SetPosition(0.0f, 0.0f, -5.0f);
    fpsCamera->SetRotation(0.0f, DirectX::XM_PIDIV2);

    orbitCamera->SetTarget(0.0f, 0.0f, 0.0f);
    orbitCamera->SetRadius(5.0f);
    orbitCamera->UpdatePosition();

    activeCamera = orbitCamera.get();
    return true;
}

void Graphics::ToggleCamera() {
    if (activeCamera == orbitCamera.get()) activeCamera = fpsCamera.get();
    else activeCamera = orbitCamera.get();
}

void Graphics::HandleInput(const InputHandler& input, float deltaTime) {
    const float moveSpeed = 5.0f * deltaTime;
    const float rotationSpeed = 0.005f *  (1.0f / (deltaTime > 0.0f ? deltaTime : 1.0f));
    // Note: rotationSpeed chosen so mouse delta (px) maps to radians sensibly
    // Alternatively you can pick a constant like 0.01f and multiply by delta.

    const float zoomSpeed = 3.0f * deltaTime;

    if (input.WasKeyPressedThisFrame(Keys::F1)) {
        ToggleCamera();
    }

    if (input.WasKeyPressedThisFrame(Keys::F2)) { currentProjection = ProjectionMode::Normal; UpdateProjection(width, height); }
    if (input.WasKeyPressedThisFrame(Keys::F3)) { currentProjection = ProjectionMode::Wide;   UpdateProjection(width, height); }
    if (input.WasKeyPressedThisFrame(Keys::F4)) { currentProjection = ProjectionMode::Narrow; UpdateProjection(width, height); }
    if (input.WasKeyPressedThisFrame(Keys::F5)) { currentProjection = ProjectionMode::Ortho;  UpdateProjection(width, height); }


    if (activeCamera == fpsCamera.get()) {
        if (input.IsMovingForward())  fpsCamera->MoveForward(moveSpeed);
        if (input.IsMovingBackward()) fpsCamera->MoveBackward(moveSpeed);
        if (input.IsMovingRight())    fpsCamera->MoveRight(moveSpeed);
        if (input.IsMovingLeft())     fpsCamera->MoveLeft(moveSpeed);
        if (input.IsMovingUp())       fpsCamera->MoveUp(moveSpeed);
        if (input.IsMovingDown())     fpsCamera->MoveDown(moveSpeed);

        if (input.IsCameraRotating()) {
            auto d = input.GetMouseDelta();
            fpsCamera->Rotate(d.x * 0.01f, d.y * 0.01f);
        }
    } else if (activeCamera == orbitCamera.get()) {
        if (input.IsCameraRotating()) {
            auto d = input.GetMouseDelta();
            orbitCamera->Rotate(d.x * 0.01f, d.y * 0.01f);
        }

        float wheelDelta = input.GetMouseWheelDelta();
        if (wheelDelta != 0.0f) {
            orbitCamera->Zoom(wheelDelta * zoomSpeed);
        }

        if (input.IsMovingForward())  orbitCamera->MoveForward(moveSpeed * 0.2f);
        if (input.IsMovingBackward()) orbitCamera->MoveBackward(moveSpeed * 0.2f);
        if (input.IsMovingRight())    orbitCamera->MoveRight(moveSpeed * 0.2f);
        if (input.IsMovingLeft())     orbitCamera->MoveLeft(moveSpeed * 0.2f);
        if (input.IsMovingUp())       orbitCamera->MoveUp(moveSpeed * 0.2f);
        if (input.IsMovingDown())     orbitCamera->MoveDown(moveSpeed * 0.2f);
    }
}

void Graphics::Update(float deltaTime, InputHandler& input) {
    HandleInput(input, deltaTime);
    scene.Update(deltaTime);
}

void Graphics::Render(float totalTime, float width, float height) {
    float color[] = {0.1f, 0.1f, 0.1f, 1.0f};
    if (renderTargetView) context->ClearRenderTargetView(renderTargetView, color);
    if (depthStencilView) context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
    context->OMSetDepthStencilState(depthStencilState, 1);
    context->RSSetState(rastState);

    context->IASetInputLayout(inputLayout);
    context->VSSetShader(vertexShader, nullptr, 0);
    context->PSSetShader(pixelShader, nullptr, 0);
    context->VSSetConstantBuffers(0, 1, &constantBuffer);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    D3D11_VIEWPORT vp = {};
    vp.Width = width;
    vp.Height = height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    context->RSSetViewports(1, &vp);

    const auto& objects = scene.GetObjects();
    for (auto& obj : objects) {
        DirectX::XMMATRIX world = DirectX::XMMatrixRotationAxis(DirectX::XMLoadFloat3(&obj.rotationAxis), obj.rotationAngle)
                       * DirectX::XMMatrixTranslation(obj.position.x, obj.position.y, obj.position.z);

        VSConstants vsConst = {
            XMMatrixTranspose(world),
            XMMatrixTranspose(activeCamera->GetViewMatrix()),
            XMMatrixTranspose(projectionMatrix)
        };

        context->UpdateSubresource(constantBuffer, 0, nullptr, &vsConst, 0, 0);

        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        context->IASetVertexBuffers(0, 1, &obj.vertexBuffer, &stride, &offset);
        context->IASetIndexBuffer(obj.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
        context->IASetPrimitiveTopology(
    obj.isOrbit ? D3D11_PRIMITIVE_TOPOLOGY_LINELIST : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
);

        context->DrawIndexed(obj.indexCount, 0, 0);
    }

    swapChain->Present(1, 0);
}

void Graphics::UpdateProjection(UINT w, UINT h) {
    float aspect = (h > 0) ? (static_cast<float>(w) / static_cast<float>(h)) : 1.0f;
    switch (currentProjection) {
        case ProjectionMode::Normal:
            projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, aspect, 0.1f, 100.0f);
            break;
        case ProjectionMode::Wide:
            projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90.0f), aspect, 0.1f, 100.0f);
            break;
        case ProjectionMode::Narrow:
            projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(30.0f), aspect, 0.1f, 100.0f);
            break;
        case ProjectionMode::Ortho:
            projectionMatrix = DirectX::XMMatrixOrthographicLH(static_cast<float>(w) / 1000.0f,
                                                      static_cast<float>(h) / 1000.0f,
                                                      0.1f, 100.0f);
            break;
    }
}
