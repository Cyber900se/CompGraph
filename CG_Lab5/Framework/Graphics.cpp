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
    if (!InitShadowMap()) return false;
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

    D3D11_BUFFER_DESC worldCbd = {};
    worldCbd.Usage = D3D11_USAGE_DEFAULT;
    worldCbd.ByteWidth = sizeof(DirectX::XMMATRIX); // Только World
    worldCbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    device->CreateBuffer(&worldCbd, nullptr, &worldConstantBuffer);

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
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, psBlob, vsBlobLight, error;

    HRESULT hr = D3DCompileFromFile(L"Shaders/MyVeryFirstShader.hlsl", nullptr, nullptr,
                                   "VSMain", "vs_5_0", 0, 0, &vsBlob, &error);
    if (FAILED(hr)) {
        if (error) {
            MessageBox(nullptr, L"Shader Error!", L"Error", MB_OK);
        }
        return false;
    }
    hr = device->CreateVertexShader(vsBlob->GetBufferPointer(),
                                vsBlob->GetBufferSize(),
                                nullptr,
                                &vertexShader);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create main vertex shader!", L"Error", MB_OK);
        return false;
    }

    hr = D3DCompileFromFile(L"Shaders/MyVeryFirstShader.hlsl", nullptr, nullptr,
                            "LightVSMain", "vs_5_0", 0, 0, &vsBlobLight, &error);
    if (FAILED(hr)) {
        if (error) MessageBox(nullptr, L"Light Shader Error!", L"Error", MB_OK);
        return false;
    }
    hr = device->CreateVertexShader(vsBlobLight->GetBufferPointer(), vsBlobLight->GetBufferSize(),
                                    nullptr, &lightVertexShader);
    if (FAILED(hr)) return false;


    hr = D3DCompileFromFile(L"Shaders/MyVeryFirstShader.hlsl", nullptr, nullptr,
                            "PSMain", "ps_5_0", 0, 0, &psBlob, &error);
    if (FAILED(hr)) return false;
    hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);
    if (FAILED(hr)) return false;

    D3D11_INPUT_ELEMENT_DESC objLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };


    hr = device->CreateInputLayout(objLayout, ARRAYSIZE(objLayout),
                                   vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"CreateInputLayout failed!", L"Error", MB_OK);
        return false;
    }

    return true;
}

bool Graphics::InitShadowMap() {
    // 1. Создание текстуры глубины
    D3D11_TEXTURE2D_DESC depthTexDesc = {};
    depthTexDesc.Width = SHADOW_MAP_SIZE;
    depthTexDesc.Height = SHADOW_MAP_SIZE;
    depthTexDesc.MipLevels = 1;
    depthTexDesc.ArraySize = 1;
    depthTexDesc.SampleDesc.Count = 1;
    depthTexDesc.Format = DXGI_FORMAT_R24G8_TYPELESS; // Используем TYPELESS для возможности создания SRV и DSV
    depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
    depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = device->CreateTexture2D(&depthTexDesc, nullptr, &shadowMapTexture);
    if (FAILED(hr)) return false;

    // 2. Создание Depth Stencil View (DSV) для рендеринга в карту теней
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // Формат глубины
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    hr = device->CreateDepthStencilView(shadowMapTexture.Get(), &dsvDesc, &shadowMapDSV);
    if (FAILED(hr)) return false;

    // 3. Создание Shader Resource View (SRV) для считывания карты теней в шейдерах
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; // Формат для чтения глубины
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    hr = device->CreateShaderResourceView(shadowMapTexture.Get(), &srvDesc, &shadowMapSRV);
    if (FAILED(hr)) return false;

    // 4. Создание Sampler State для считывания карты теней (с компаратором для PCF)
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT; // Сравнение для теней
    samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.BorderColor[0] = samplerDesc.BorderColor[1] = samplerDesc.BorderColor[2] = samplerDesc.BorderColor[3] = 1.0f; // За пределами текстуры — тень (глубина 1)
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;

    hr = device->CreateSamplerState(&samplerDesc, &shadowSampler);
    if (FAILED(hr)) return false;

    // 5. Создание константного буфера для матриц света
    D3D11_BUFFER_DESC lightCbd = {};
    lightCbd.Usage = D3D11_USAGE_DEFAULT;
    lightCbd.ByteWidth = sizeof(DirectX::XMMATRIX) * 2; // LightView * LightProj
    lightCbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    hr = device->CreateBuffer(&lightCbd, nullptr, &lightConstantBuffer);

    // НОВОЕ: 6. Создание Rasterizer State для карты теней (Shadow Bias)
    D3D11_RASTERIZER_DESC shadowRastDesc = {};
    shadowRastDesc.FillMode = D3D11_FILL_SOLID;
    shadowRastDesc.CullMode = D3D11_CULL_FRONT; // Отбрасываем лицевые грани
    shadowRastDesc.DepthClipEnable = TRUE;

    // !!! Настройка Depth Bias для борьбы с Shadow Acne !!!
    // Настройте эти значения, если будут артефакты.
    shadowRastDesc.DepthBias = 1000;
    shadowRastDesc.DepthBiasClamp = 0.0f;
    shadowRastDesc.SlopeScaledDepthBias = 2.0f;

    hr = device->CreateRasterizerState(&shadowRastDesc, &shadowMapRasterizerState);
    if (FAILED(hr)) return false;

    return SUCCEEDED(hr);
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

    // Упрощенный направленный свет, смотрящий в центр (0,0,0)
    DirectX::XMVECTOR lightDir = DirectX::XMVector3Normalize(DirectX::XMVectorSet(1.0f, -1.0f, 1.0f, 0.0f));
    DirectX::XMVECTOR target = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR lightPos = DirectX::XMVectorSubtract(target, DirectX::XMVectorScale(lightDir, 15.0f));
    up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    lightViewMatrix = DirectX::XMMatrixLookAtLH(lightPos, target, up);

    // Ортографическая проекция, охватывающая сцену (например, 20x20)
    float sceneBound = 1000.0f;
    lightProjectionMatrix = DirectX::XMMatrixOrthographicLH(sceneBound * 2.0f, sceneBound * 2.0f, 0.1f, 1000.0f);

    // Сохраняем матрицы в константный буфер для света.
    // Нам это понадобится и для первого (VS) и для второго (VS) прохода.
    struct LightMatricesData
    {
        DirectX::XMMATRIX LightView;
        DirectX::XMMATRIX LightProjection;
    };

    // обновляем
    LightMatricesData lm{};
    lm.LightView = XMMatrixTranspose(lightViewMatrix);
    lm.LightProjection = XMMatrixTranspose(lightProjectionMatrix);

    context->UpdateSubresource(lightConstantBuffer.Get(), 0, nullptr, &lm, 0, 0);
}

void Graphics::RenderShadowMap() {
    // 1. Настройка Viewport для карты теней
    D3D11_VIEWPORT shadowVp = {};
    shadowVp.Width = static_cast<float>(SHADOW_MAP_SIZE);
    shadowVp.Height = static_cast<float>(SHADOW_MAP_SIZE);
    shadowVp.MinDepth = 0.0f;
    shadowVp.MaxDepth = 1.0f;
    context->RSSetViewports(1, &shadowVp);
    context->RSSetState(shadowMapRasterizerState);

    // 2. Очистка DSV карты теней
    context->ClearDepthStencilView(shadowMapDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
    context->OMSetRenderTargets(0, nullptr, shadowMapDSV);

    // 3. Установка цели рендеринга: ТОЛЬКО DSV (RenderTargetView = nullptr)
    context->OMSetRenderTargets(0, nullptr, shadowMapDSV);

    // 4. Установка шейдеров для рендеринга глубины
    context->IASetInputLayout(inputLayout);
    context->VSSetShader(lightVertexShader, nullptr, 0); // LightVS
    context->PSSetShader(nullptr, nullptr, 0); // Pixel Shader не нужен!

    // 5. Установка константных буферов
    context->VSSetConstantBuffers(3, 1, lightConstantBuffer.GetAddressOf());

    // ИЗМЕНЕНИЕ СЛОТА: WorldOnly в слот 0 (b0)
    context->VSSetConstantBuffers(0, 1, worldConstantBuffer.GetAddressOf());


    // --- Функция отрисовки для первого прохода ---
    auto drawObjectDepth = [&](const RenderObject& obj, const DirectX::XMMATRIX& world) {
        DirectX::XMMATRIX worldTransposed = DirectX::XMMatrixTranspose(world);
        // Обновляем новый буфер, содержащий только World
        context->UpdateSubresource(worldConstantBuffer.Get(), 0, nullptr, &worldTransposed, 0, 0);

        UINT stride = sizeof(Vertex), offset = 0;
        context->IASetVertexBuffers(0, 1, &obj.vertexBuffer, &stride, &offset);
        context->IASetIndexBuffer(obj.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->DrawIndexed(obj.indexCount, 0, 0);
    };

    // Статические
    for (const auto& obj : scene.GetStaticObjects()) {
        DirectX::XMMATRIX world = DirectX::XMMatrixTranslation(obj.position.x, obj.position.y, obj.position.z);
        drawObjectDepth(obj, world);
    }

    // Динамические
    for (auto& obj : scene.GetDynamicObjects()) {
        DirectX::XMMATRIX rot = XMLoadFloat4x4(&obj.rotationMatrix);
        DirectX::XMMATRIX trans = DirectX::XMMatrixTranslation(obj.position.x, obj.position.y, obj.position.z);
        DirectX::XMMATRIX world = rot * trans;
        drawObjectDepth(obj, world);
    }

    // Игрок
    {
        auto& p = scene.player.sphere;
        DirectX::XMMATRIX rot = XMLoadFloat4x4(&p.rotationMatrix);
        DirectX::XMMATRIX trans = DirectX::XMMatrixTranslation(p.position.x, p.position.y, p.position.z);
        DirectX::XMMATRIX world = rot * trans;
        drawObjectDepth(p, world);
    }


    // 7. Сброс константных буферов и ресурсов
    ID3D11Buffer* nullCBs[4] = { nullptr, nullptr, nullptr, nullptr };
    context->VSSetConstantBuffers(0, 4, nullCBs); // Сброс слотов 0, 1, 2, 3

    ID3D11DepthStencilView* nullDSV = nullptr;
    context->OMSetRenderTargets(0, nullptr, nullDSV);

    // НОВОЕ: Возврат к основному растеризатору
    context->RSSetState(rastState);
}

void Graphics::Render(float totalTime, float width, float height, std::vector<DirectX::XMFLOAT3> lightPositions) {
    RenderShadowMap();
    // Получаем позицию камеры
    DirectX::XMFLOAT3 camPos{};
    DirectX::XMStoreFloat3(&camPos, activeCamera->GetPositionVector());

    // Заполняем константный буфер для пиксельного шейдера
    PSConstants psConst = {};
    psConst.ambientColor = DirectX::XMFLOAT3(0.1f, 0.1f, 0.1f);
    psConst.numLights = static_cast<int>(lightPositions.size());
    psConst.cameraPos = camPos;

    // Инициализация источников света
    for (int i = 0; i < psConst.numLights; i++) {
        psConst.lights[i].position = lightPositions[i];
        psConst.lights[i].color = { 1.0f, 1.0f, 1.0f };
        psConst.lights[i].intensity = 0.01f; // достаточно ярко
    }

    context->UpdateSubresource(psConstantBuffer, 0, nullptr, &psConst, 0, 0);
    context->PSSetConstantBuffers(2, 1, &psConstantBuffer);

    // Очистка буфера
    float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    context->ClearRenderTargetView(renderTargetView, clearColor);
    context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

    D3D11_VIEWPORT vp{};
    vp.Width = width;
    vp.Height = height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    context->RSSetViewports(1, &vp);

    context->PSSetShaderResources(0, 1, shadowMapSRV.GetAddressOf());
    context->PSSetSamplers(0, 1, shadowSampler.GetAddressOf());

    context->IASetInputLayout(inputLayout);
    context->VSSetShader(vertexShader, nullptr, 0);
    context->PSSetShader(pixelShader, nullptr, 0);
    context->VSSetConstantBuffers(1, 1, &constantBuffer);
    context->VSSetConstantBuffers(3, 1, lightConstantBuffer.GetAddressOf());

    DirectX::XMMATRIX view = activeCamera->GetViewMatrix();
    DirectX::XMMATRIX proj = projectionMatrix;

    auto drawObject = [&](const RenderObject& obj, const DirectX::XMMATRIX& world) {
        DirectX::XMMATRIX worldInvTranspose = XMMatrixTranspose(XMMatrixInverse(nullptr, world));

        VSConstants vsConst{
            XMMatrixTranspose(world),
            XMMatrixTranspose(view),
            XMMatrixTranspose(proj),
            XMMatrixTranspose(worldInvTranspose)
        };
        context->UpdateSubresource(constantBuffer, 0, nullptr, &vsConst, 0, 0);

        UINT stride = sizeof(Vertex), offset = 0;
        context->IASetVertexBuffers(0, 1, &obj.vertexBuffer, &stride, &offset);
        context->IASetIndexBuffer(obj.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->DrawIndexed(obj.indexCount, 0, 0);
    };

    for (const auto& obj : scene.GetStaticObjects()) {
        DirectX::XMMATRIX world = DirectX::XMMatrixTranslation(obj.position.x, obj.position.y, obj.position.z);
        drawObject(obj, world);
    }

    for (auto& obj : scene.GetDynamicObjects()) {
        // Загружаем матрицу вращения (обновленную в Scene::Update)
        DirectX::XMMATRIX rot = XMLoadFloat4x4(&obj.rotationMatrix);

        // Создаем матрицу перемещения
        DirectX::XMMATRIX trans = DirectX::XMMatrixTranslation(obj.position.x, obj.position.y, obj.position.z);

        // Комбинируем: World = Rotation * Translation (для правильного порядка преобразований)
        DirectX::XMMATRIX world = rot * trans;

        drawObject(obj, world);
    }

    {
        auto& p = scene.player.sphere;
        DirectX::XMMATRIX rot = XMLoadFloat4x4(&p.rotationMatrix);
        DirectX::XMMATRIX trans = DirectX::XMMatrixTranslation(p.position.x, p.position.y, p.position.z);
        DirectX::XMMATRIX world = rot * trans;
        drawObject(p, world);
    }
    ID3D11ShaderResourceView* nullSRV[1] = {nullptr};
    context->PSSetShaderResources(0, 1, nullSRV);


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