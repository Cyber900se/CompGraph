#include "Graphics.h"
#include "Geometry/RenderObject.h"

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

    cbd = {};
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.ByteWidth = sizeof(PSConstants); // обязательно размер структуры для пиксельного шейдера
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = 0;
    cbd.MiscFlags = 0;
    cbd.StructureByteStride = 0;

    hr = device->CreateBuffer(&cbd, nullptr, &psConstantBuffer);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create PS constant buffer!", L"Error", MB_OK);
        return false; // или другой способ обработки ошибки
    }
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
        {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };


    device->CreateInputLayout(objLayout, ARRAYSIZE(objLayout), vsBlob->GetBufferPointer(),
                              vsBlob->GetBufferSize(), &inputLayout);

    return true;
}

bool Graphics::InitCamera() {
    thirdPersonCamera = std::make_unique<ThirdPersonCamera>();
    thirdPersonCamera->SetTarget(scene.player.sphere.position);
    activeCamera = thirdPersonCamera.get();
    return true;
}

void Graphics::HandleInput(const InputHandler& input, float deltaTime) {
    // вращение камеры мышью
    if (activeCamera == thirdPersonCamera.get()) {
        const float rotationSpeed = 0.01f;
        auto d = input.GetMouseDelta();
        thirdPersonCamera->Update(scene.player.sphere.position, d.x * rotationSpeed, d.y * rotationSpeed);
    }
}

void Graphics::Update(float deltaTime, InputHandler& input)
{
    const float rotationSpeed = 0.01f;

    // обновляем камеру по мыши (если третье лицо)
    if (activeCamera == thirdPersonCamera.get()) {
        auto d = input.GetMouseDelta();
        thirdPersonCamera->Update(scene.player.sphere.position, d.x * rotationSpeed, d.y * rotationSpeed);
    }

    // получаем мировые векторы камеры (проекция на XZ)
    DirectX::XMVECTOR camPos    = DirectX::XMLoadFloat3(&thirdPersonCamera->camPos);
    DirectX::XMVECTOR camTarget = DirectX::XMLoadFloat3(&thirdPersonCamera->camTarget);

    DirectX::XMVECTOR camForward = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(camTarget, camPos));
    // проекция на горизонтальную плоскость
    camForward = DirectX::XMVectorSetY(camForward, 0.0f);
    camForward = DirectX::XMVector3Normalize(camForward);

    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    // ВАЖНО: порядок аргументов здесь определяет направление "вправо".
    // Для DirectX (левая система) правильный right = cross(up, forward)
    DirectX::XMVECTOR camRight =
    DirectX::XMVector3Normalize(DirectX::XMVector3Cross(camForward, up));

    // передаём вычисленные направления в сцену
    scene.Update(deltaTime, input, camForward, camRight);
}


void Graphics::Render(float totalTime, float width, float height, std::vector<DirectX::XMFLOAT3> lightPositions) {
    DirectX::XMVECTOR camPosVec = activeCamera->GetPositionVector(); // метод, который возвращает XMVECTOR позиции камеры
    DirectX::XMFLOAT3 camPos{};
    DirectX::XMStoreFloat3(&camPos, camPosVec);

    PSConstants psConst = {};
    psConst.numLights = 100;
    psConst.ambientColor = DirectX::XMFLOAT3(0.1f, 0.1f, 0.1f);
    psConst.lights[0].position = DirectX::XMFLOAT3(0, 5, -5);
    psConst.lights[0].intensity = 0.1f;
    psConst.lights[0].color = DirectX::XMFLOAT3(1, 1, 1);

    context->UpdateSubresource(psConstantBuffer, 0, nullptr, &psConst, 0, 0);

    for(int i = 0;i < psConst.numLights; i++){
        psConst.lights[i].position = lightPositions[i];
        psConst.lights[i].color = {1.0f,1.0f,1.0f};
        psConst.lights[i].intensity = 0.01f;
    }

    context->UpdateSubresource(psConstantBuffer, 0, nullptr, &psConst, 0, 0);
    context->PSSetConstantBuffers(1, 1, &psConstantBuffer);


    float clearColor[] = {0.1f,0.1f,0.1f,1.0f};
    context->ClearRenderTargetView(renderTargetView, clearColor);
    context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

    context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

    D3D11_VIEWPORT vp{};
    vp.Width = width;
    vp.Height = height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    context->RSSetViewports(1, &vp);

    context->IASetInputLayout(inputLayout);
    context->VSSetShader(vertexShader, nullptr, 0);
    context->PSSetShader(pixelShader, nullptr, 0);
    context->VSSetConstantBuffers(0, 1, &constantBuffer);

    // --- Матрицы камеры ---
    DirectX::XMMATRIX view = activeCamera->GetViewMatrix();
    DirectX::XMMATRIX proj = projectionMatrix;

    auto drawObject = [&](const RenderObject &obj, const DirectX::XMMATRIX &world) {
        VSConstants vsConst {
            DirectX::XMMatrixTranspose(world),
            DirectX::XMMatrixTranspose(view),
            DirectX::XMMatrixTranspose(proj),
            DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, world)) // inverse-transpose
        };
        context->UpdateSubresource(constantBuffer, 0, nullptr, &vsConst, 0, 0);

        UINT stride = sizeof(Vertex), offset = 0;
        context->IASetVertexBuffers(0, 1, &obj.vertexBuffer, &stride, &offset);
        context->IASetIndexBuffer(obj.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->DrawIndexed(obj.indexCount, 0, 0);
    };

    // --- Отрисовка статических объектов ---
    for (const RenderObject obj : scene.GetStaticObjects()) {
        DirectX::XMMATRIX world =
            DirectX::XMMatrixTranslation(obj.position.x, obj.position.y, obj.position.z);
        drawObject(obj, world);
    }

    // --- Отрисовка динамических объектов ---
    for (auto &obj : scene.GetDynamicObjects()) {
        DirectX::XMMATRIX world =
            DirectX::XMMatrixTranslation(obj.position.x, obj.position.y, obj.position.z);
        drawObject(obj, world);
    }

    // --- Отрисовка игрока ---
    {
        auto &p = scene.player.sphere;
        DirectX::XMMATRIX rot = DirectX::XMLoadFloat4x4(&p.rotationMatrix);
        DirectX::XMMATRIX trans = DirectX::XMMatrixTranslation(p.position.x, p.position.y, p.position.z);
        DirectX::XMMATRIX world = rot * trans; // сначала вращение, потом смещение
        drawObject(p, world);
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
            projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90.0f), aspect, 0.1f, 100.0f); break;
        case ProjectionMode::Narrow: projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(30.0f), aspect, 0.1f, 100.0f);
            break;
        case ProjectionMode::Ortho: projectionMatrix = DirectX::XMMatrixOrthographicLH(static_cast<float>(w) / 1000.0f, static_cast<float>(h) / 1000.0f, 0.1f, 100.0f);
            break;
    }
}