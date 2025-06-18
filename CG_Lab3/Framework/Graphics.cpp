#include "Graphics.h"
#include <algorithm>
#include <fstream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

struct Vertex {
    DirectX::XMFLOAT4 pos;
    DirectX::XMFLOAT4 col;
};

struct BackgroundVertex {
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT2 uv;
};

struct Graphics::RenderObject {
    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;
    UINT indexCount;
    DirectX::XMFLOAT2 position;
    DirectX::XMFLOAT2 velocity;
};

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

    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.ByteWidth = sizeof(VSConstants);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = 0;

    HRESULT hr = device->CreateBuffer(&cbd, nullptr, &constantBuffer);
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

void Graphics::CreateBackground() {
    BackgroundVertex bgVertices[] = {
        { {-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f} },
        { { 1.0f,  1.0f, 0.0f}, {1.0f, 0.0f} },
        { {-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f} },

        { { 1.0f,  1.0f, 0.0f}, {1.0f, 0.0f} },
        { { 1.0f, -1.0f, 0.0f}, {1.0f, 1.0f} },
        { {-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f} },
    };

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.ByteWidth = sizeof(bgVertices);
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = bgVertices;

    HRESULT hr = device->CreateBuffer(&vbDesc, &initData, &backgroundVB);
    if (FAILED(hr)) {
        // log или throw
    }

    backgroundVertexCount = 6;
}

bool Graphics::InitShaders(HWND hWnd) {
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, psBlob, error;
    &vsBlob = nullptr;
    HRESULT hr = D3DCompileFromFile(L"Shaders/ChessboardShader.hlsl", nullptr, nullptr,
        "VS_Main", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vsBlob, &error);
    if (FAILED(hr)) {
        if (error) {
            OutputDebugStringA((char*)error->GetBufferPointer()); // отладочный вывод
            std::ofstream fout("shader-error.txt");
            fout << (char*)error->GetBufferPointer();
            fout.close();
        }
        MessageBox(hWnd, L"Shader compilation failed. Check shader-error.txt", L"Error", MB_OK);
        return false;
    }
    device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &backgroundVS);

    psBlob = nullptr;
    hr = D3DCompileFromFile(L"Shaders/ChessboardShader.hlsl", nullptr, nullptr,
        "PS_Main", "ps_5_0", 0, 0, &psBlob, &error);
    if (FAILED(hr)) {
        MessageBox(0, L"Failed to compile Background PS", L"Error", MB_OK);
        return 0;
    }
    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &backgroundPS);

    D3D11_INPUT_ELEMENT_DESC bgLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    device->CreateInputLayout(bgLayout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &backgroundLayout);

    D3DCompileFromFile(L"Shaders/MyVeryFirstShader.hlsl", nullptr, nullptr,
        "VSMain", "vs_5_0", 0, 0, &vsBlob, &error);
    device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &objectVS);

    D3DCompileFromFile(L"Shaders/MyVeryFirstShader.hlsl", nullptr, nullptr,
        "PSMain", "ps_5_0", 0, 0, &psBlob, &error);
    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &objectPS);

    D3D11_INPUT_ELEMENT_DESC objLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    device->CreateInputLayout(objLayout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &objectLayout);

    return true;
}

Graphics::RenderObject Graphics::CreateRectangleMesh(float meshWidth, float meshHeight, DirectX::XMFLOAT4 color) {
    float w = meshWidth / 2;
    float h = meshHeight / 2;

    Vertex vertices[] = {
        { {-w, -h, 0.5f, 1}, color },
        { { w, -h, 0.5f, 1}, color },
        { { w,  h, 0.5f, 1}, color },
        { {-w,  h, 0.5f, 1}, color },
    };

    UINT indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    RenderObject obj;
    obj.indexCount = 6;
    D3D11_BUFFER_DESC vertexBufDesc = {};
    vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufDesc.ByteWidth = sizeof(Vertex) * std::size(vertices);
    vertexBufDesc.CPUAccessFlags = 0;
    vertexBufDesc.MiscFlags = 0;
    vertexBufDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;
    HRESULT hr = device->CreateBuffer(&vertexBufDesc, &vertexData, &obj.vertexBuffer);
    if (FAILED(hr)) {
        std::cerr << "Failed to create vertex buffer for rectangle. HRESULT: " << std::hex << hr << std::endl;
    }

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

    hr = device->CreateBuffer(&indexBufDesc, &indexData, &obj.indexBuffer);
    if (FAILED(hr)) {
        std::cerr << "Failed to create index buffer for rectangle. HRESULT: " << std::hex << hr << std::endl;
    }
    context->ClearState();

    return obj;
}

Graphics::RenderObject Graphics::CreateCircleMesh(float meshSegments, float meshRadius) {

    std::vector<Vertex> vertices;
    std::vector<UINT> indices;

    // Центр круга
    Vertex centerVertex = {
        { 0.0f, 0.0f, 0.5f, 1.0f },
        { 1.0f, 1.0f, 1.0f, 1.0f }
    };
    vertices.push_back(centerVertex);

    // Вершины по окружности
    for (int i = 0; i <= meshSegments; ++i) {
        float angle = DirectX::XM_2PI * i / meshSegments;
        float x = cosf(angle) * meshRadius;
        float y = sinf(angle) * meshRadius;

        Vertex v;
        v = {
            { x, y, 0.5f, 1.0f },
            { 0.5f + 0.5f * cosf(angle), 0.5f + 0.5f * sinf(angle), 1.0f, 1.0f}
        };
        vertices.push_back(v);
    }

    for (UINT i = 1; i <= meshSegments; ++i) {
        indices.push_back(0);        // центр
        indices.push_back(i);
        indices.push_back(i + 1 <= meshSegments ? i + 1 : 1); // wrap
    }

    RenderObject obj;
    D3D11_BUFFER_DESC vertexBufDesc = {};
    vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufDesc.ByteWidth = UINT(vertices.size() * sizeof(Vertex));
    vertexBufDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = vertices.data();

    HRESULT hr = device->CreateBuffer(&vertexBufDesc, &vertexData, &obj.vertexBuffer);
    if (FAILED(hr)) {
        std::cerr << "Failed to create vertex buffer for circle. HRESULT: " << std::hex << hr << std::endl;
    }

    D3D11_BUFFER_DESC indexBufDesc = {};
    indexBufDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufDesc.ByteWidth = static_cast<UINT>(indices.size() * sizeof(UINT));
    indexBufDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices.data();

    device->CreateBuffer(&indexBufDesc, &indexData, &obj.indexBuffer);
    if (FAILED(hr)) {
        std::cerr << "Failed to create index buffer for circle. HRESULT: " << std::hex << hr << std::endl;
    }

    obj.indexCount = static_cast<UINT>(indices.size());

    context->ClearState();
    return obj;
}

bool Graphics::InitGeometry() {
    CreateBackground();
    RenderObject ball = CreateCircleMesh(32, 0.1f);
    ball.position = { 0.0f, 0.0f };
    ball.velocity = {0.5f, 0.0f};

    ResetBall();

    RenderObject leftRect = CreateRectangleMesh(0.1f, 0.4f, {0, 1, 0, 1});
    leftRect.position = { -0.8f, 0.0f };

    RenderObject rightRect = CreateRectangleMesh(0.1f, 0.4f, {0, 0, 1, 1});
    rightRect.position = { 0.8f, 0.0f };

    objects.push_back(ball);
    objects.push_back(leftRect);
    objects.push_back(rightRect);

    leftPlayerScore = 0;
    rightPlayerScore = 0;

    return true;
}

void Graphics::MoveLeftPaddle(float dy) {
    if (objects.size() > 1) {
        objects[1].position.y += dy;
        objects[1].position.y = std::clamp(objects[1].position.y, -0.8f, 0.8f);
    }
}

void Graphics::MoveRightPaddle(float dy) {
    if (objects.size() > 2) {
        objects[2].position.y += dy;
        objects[2].position.y = std::clamp(objects[2].position.y, -0.8f, 0.8f);
    }
}

void Graphics::Update(float deltaTime) {
    if (objects.empty()) return;
    const float maxAngleDeg = 30.0f;
    const float maxAngleRad = maxAngleDeg * (3.14159f / 180.0f);
    float verticalFactor = tanf(maxAngleRad);

    RenderObject& ball = objects[0];
    const float ballSpeed = 1.5f * deltaTime;

    ball.position.x += ball.velocity.x * ballSpeed;
    ball.position.y += ball.velocity.y * ballSpeed;


    const float boundary = 1.0f;
    const float ballRadius = 0.1f;

    if (ball.position.y + ballRadius > boundary ||
        ball.position.y - ballRadius < -boundary) {
        ball.velocity.y *= -1;
        }

    constexpr float boundaryX = 1.1f;
    if (ball.position.x + ballRadius > boundaryX) {
        leftPlayerScore++;
        ResetBall();
    } else if (ball.position.x - ballRadius < -boundaryX) {
        rightPlayerScore++;
        ResetBall();
    }

    if (objects.size() > 2) {
        RenderObject& leftPaddle = objects[1];
        RenderObject& rightPaddle = objects[2];

        const float paddleHalfWidth = 0.05f;
        const float paddleHalfHeight = 0.2f;

        if (ball.position.x - ballRadius < leftPaddle.position.x + paddleHalfWidth &&
            ball.position.x + ballRadius > leftPaddle.position.x - paddleHalfWidth &&
            ball.position.y - ballRadius < leftPaddle.position.y + paddleHalfHeight &&
            ball.position.y + ballRadius > leftPaddle.position.y - paddleHalfHeight) {

            ball.velocity.x = fabs(ball.velocity.x);
            float hitY = (ball.position.y - leftPaddle.position.y) / paddleHalfHeight;

            ball.velocity.y = hitY * 0.8f * verticalFactor;
            }

        if (ball.position.x - ballRadius < rightPaddle.position.x + paddleHalfWidth &&
            ball.position.x + ballRadius > rightPaddle.position.x - paddleHalfWidth &&
            ball.position.y - ballRadius < rightPaddle.position.y + paddleHalfHeight &&
            ball.position.y + ballRadius > rightPaddle.position.y - paddleHalfHeight) {

            ball.velocity.x = -fabs(ball.velocity.x);
            float hitY = (ball.position.y - rightPaddle.position.y) / paddleHalfHeight;

            ball.velocity.y = hitY * 0.8f * verticalFactor;
            }
    }
}

void Graphics::ResetBall() {
    if (objects.empty()) return;

    RenderObject& ball = objects[0];
    ball.position = {0.0f, 0.0f};

    ball.velocity.y = 0.0f;

    bool moveRight = (rand() % 2 == 0);

    const float horizontalSpeed = 0.5f;
    ball.velocity.x = moveRight ? horizontalSpeed : -horizontalSpeed;
}

void Graphics::Render(float totalTime, float width, float height) {

    context->RSSetState(rastState);

    DirectX::XMVECTOR eyePos = DirectX::XMVectorSet(0.0f, 0.0f, -7.0f, 1.0f);     // Камера немного позади объекта по Z
    DirectX::XMVECTOR focusPos = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);   // Смотрит на центр
    DirectX::XMVECTOR upDir = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);       // Вверх — по Y

    D3D11_VIEWPORT viewport = {};
    viewport.Width = width;
    viewport.Height = height;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1.0f;
    context->RSSetViewports(1, &viewport);

    context->VSSetConstantBuffers(0, 1, &constantBuffer);
    context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

    float color[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    context->ClearRenderTargetView(renderTargetView, color);
    context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    context->OMSetDepthStencilState(depthStencilState, 1); // Включаем тест глубины
    context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
    context->RSSetState(rastState);

    context->IASetInputLayout(backgroundLayout);
    context->VSSetShader(backgroundVS, nullptr, 0);
    context->PSSetShader(backgroundPS, nullptr, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    UINT stride = sizeof(BackgroundVertex);  // pos (float3) + uv (float2)
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &backgroundVB, &stride, &offset);

    context->Draw(backgroundVertexCount, 0);

    context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    context->IASetInputLayout(objectLayout);
    context->VSSetShader(objectVS, nullptr, 0);
    context->PSSetShader(objectPS, nullptr, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->VSSetConstantBuffers(0, 1, &constantBuffer);

    DirectX::XMMATRIX proj = DirectX::XMMatrixOrthographicLH(2.0f, 2.0f, 0.1f, 10.0f);

    for (RenderObject& obj : objects) {

        DirectX::XMMATRIX world = DirectX::XMMatrixTranslation(obj.position.x, obj.position.y, 0.0f);
        DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(eyePos, focusPos, upDir);

        VSConstants vsConst = {
            XMMatrixTranspose(world),
            XMMatrixTranspose(view),
            XMMatrixTranspose(proj)
        };
        context->UpdateSubresource(constantBuffer, 0, nullptr, &vsConst, 0, 0);

        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        context->IASetVertexBuffers(0, 1, &obj.vertexBuffer, &stride, &offset);
        context->IASetIndexBuffer(obj.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        context->DrawIndexed(obj.indexCount, 0, 0);
    }

    swapChain->Present(1, 0);
}
