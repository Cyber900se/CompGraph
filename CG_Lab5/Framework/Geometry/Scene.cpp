#include "Scene.h"

#include <algorithm>
#include <cmath>
#include <ctime>
#include <cstdlib>

class Camera;

RenderObject Scene::CreateCube(ID3D11Device* device, float size, const DirectX::XMFLOAT4& color,
                               float orbitRadius, float rotationSpeed, int parentIndex, float yOffset)
{
    float h = size / 2.0f;

    Vertex vertices[] = {
        // Front face
        {{-h, -h,  h, 1}, color, {0, 0, 1}},
        {{ h, -h,  h, 1}, color, {0, 0, 1}},
        {{ h,  h,  h, 1}, color, {0, 0, 1}},
        {{-h,  h,  h, 1}, color, {0, 0, 1}},

        // Back face
        {{ h, -h, -h, 1}, color, {0, 0, -1}},
        {{-h, -h, -h, 1}, color, {0, 0, -1}},
        {{-h,  h, -h, 1}, color, {0, 0, -1}},
        {{ h,  h, -h, 1}, color, {0, 0, -1}},

        // Left face
        {{-h, -h, -h, 1}, color, {-1, 0, 0}},
        {{-h, -h,  h, 1}, color, {-1, 0, 0}},
        {{-h,  h,  h, 1}, color, {-1, 0, 0}},
        {{-h,  h, -h, 1}, color, {-1, 0, 0}},

        // Right face
        {{ h, -h,  h, 1}, color, {1, 0, 0}},
        {{ h, -h, -h, 1}, color, {1, 0, 0}},
        {{ h,  h, -h, 1}, color, {1, 0, 0}},
        {{ h,  h,  h, 1}, color, {1, 0, 0}},

        // Top face
        {{-h,  h,  h, 1}, color, {0, 1, 0}},
        {{ h,  h,  h, 1}, color, {0, 1, 0}},
        {{ h,  h, -h, 1}, color, {0, 1, 0}},
        {{-h,  h, -h, 1}, color, {0, 1, 0}},

        // Bottom face
        {{-h, -h, -h, 1}, color, {0, -1, 0}},
        {{ h, -h, -h, 1}, color, {0, -1, 0}},
        {{ h, -h,  h, 1}, color, {0, -1, 0}},
        {{-h, -h,  h, 1}, color, {0, -1, 0}},
    };

    UINT indices[] = {
        0,1,2, 0,2,3,       // Front
        4,5,6, 4,6,7,       // Back
        8,9,10, 8,10,11,    // Left
        12,13,14, 12,14,15, // Right
        16,17,18, 16,18,19, // Top
        20,21,22, 20,22,23  // Bottom
    };

    // Создание буферов
    D3D11_BUFFER_DESC vbd{};
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = sizeof(vertices);
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vinit{ vertices };
    ID3D11Buffer* vbuf = nullptr;
    device->CreateBuffer(&vbd, &vinit, &vbuf);

    D3D11_BUFFER_DESC ibd{};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = sizeof(indices);
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA iinit{ indices };
    ID3D11Buffer* ibuf = nullptr;
    device->CreateBuffer(&ibd, &iinit, &ibuf);

    RenderObject obj{};
    obj.vertexBuffer = vbuf;
    obj.indexBuffer = ibuf;
    obj.indexCount = ARRAYSIZE(indices);
    obj.position = { orbitRadius + yOffset, yOffset, 0.0f };

    return obj;
}


RenderObject Scene::CreateSphere(
    ID3D11Device* device,
    float radius,
    int sliceCount,      // sectorCount (longitudes)
    int stackCount,      // stacks (latitudes) - must be >= 2
    const DirectX::XMFLOAT4& color,
    float orbitRadius,
    float rotationSpeed,
    int parentIndex,
    float yOffset)
{
    // safety
    if (sliceCount < 3) sliceCount = 3;
    if (stackCount < 2) stackCount = 2;

    std::vector<Vertex> vertices;
    std::vector<UINT> indices;

    const int sectorCount = sliceCount;
    const int stacks = stackCount;

    // Top pole
    vertices.push_back({ {0.0f,  radius, 0.0f, 1.0f}, color, {0.0f, 1.0f, 0.0f} });

    // Rings (exclude poles)
    for (int i = 1; i <= stacks - 1; ++i) // i from 1 .. stacks-1 inclusive
    {
        float phi = DirectX::XM_PI * i / stacks; // 0..PI
        float y   = radius * cosf(phi);          // y coordinate
        float r   = radius * sinf(phi);          // radius of current ring

        for (int j = 0; j < sectorCount; ++j)
        {
            float theta = 2.0f * DirectX::XM_PI * j / sectorCount;
            float x = r * cosf(theta);
            float z = r * sinf(theta);

            DirectX::XMFLOAT3 normal = { x / radius, y / radius, z / radius }; // outward normal
            vertices.push_back({ {x, y, z, 1.0f}, color, normal });
        }
    }

    // Bottom pole
    vertices.push_back({ {0.0f, -radius, 0.0f, 1.0f}, color, {0.0f, -1.0f, 0.0f} });

    // Indices
    const int topIndex = 0;
    const int baseIndex = 1; // first vertex of first ring
    const int ringCount = stacks - 1; // number of rings (excluding poles)
    const int southIndex = static_cast<int>(vertices.size()) - 1;

    // --- Top cap ---
    for (int j = 0; j < sectorCount; ++j)
    {
        // triangle: top, next, current  (winding chosen to face outward)
        indices.push_back(topIndex);
        indices.push_back(baseIndex + (j + 1) % sectorCount);
        indices.push_back(baseIndex + j);
    }

    // --- Middle (quads split into two triangles) ---
    // iterate over pairs of adjacent rings
    for (int i = 0; i < ringCount - 1; ++i)
    {
        int curBase = baseIndex + i * sectorCount;
        int nextBase = baseIndex + (i + 1) * sectorCount;

        for (int j = 0; j < sectorCount; ++j)
        {
            int cur = curBase + j;
            int next = nextBase + j;
            int curNext = curBase + (j + 1) % sectorCount;
            int nextNext = nextBase + (j + 1) % sectorCount;

            // triangle 1
            indices.push_back(cur);
            indices.push_back(curNext);
            indices.push_back(next);

            // triangle 2
            indices.push_back(curNext);
            indices.push_back(nextNext);
            indices.push_back(next);
        }
    }

    // --- Bottom cap ---
    int lastRingBase = baseIndex + (ringCount - 1) * sectorCount;
    for (int j = 0; j < sectorCount; ++j)
    {
        // triangle: bottom, current, next  (winding chosen to face outward)
        indices.push_back(southIndex);
        indices.push_back(lastRingBase + j);
        indices.push_back(lastRingBase + (j + 1) % sectorCount);
    }

    // --- Create GPU buffers ---
    D3D11_BUFFER_DESC vbd{};
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertices.size());
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vinit{};
    vinit.pSysMem = vertices.data();
    ID3D11Buffer* vbuf = nullptr;
    HRESULT hr = device->CreateBuffer(&vbd, &vinit, &vbuf);
    if (FAILED(hr)) {
        // handle error (return empty RenderObject)
        RenderObject empty{};
        return empty;
    }

    D3D11_BUFFER_DESC ibd{};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = static_cast<UINT>(sizeof(UINT) * indices.size());
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA iinit{};
    iinit.pSysMem = indices.data();
    ID3D11Buffer* ibuf = nullptr;
    hr = device->CreateBuffer(&ibd, &iinit, &ibuf);
    if (FAILED(hr)) {
        if (vbuf) vbuf->Release();
        RenderObject empty{};
        return empty;
    }

    RenderObject obj{};
    obj.vertexBuffer = vbuf;
    obj.indexBuffer = ibuf;
    obj.indexCount = static_cast<UINT>(indices.size());
    obj.position = { orbitRadius + yOffset, yOffset, 0.0f };
    obj.radius = radius;

    return obj;
}

RenderObject Scene::CreatePlane(ID3D11Device* device, float size, const DirectX::XMFLOAT4& color)
{
    float half=size/2.0f;
    Vertex vertices[] = {
        {{-half, 0, -half, 1}, color, {0, 1, 0}},
        {{ half, 0, -half, 1}, color, {0, 1, 0}},
        {{ half, 0,  half, 1}, color, {0, 1, 0}},
        {{-half, 0,  half, 1}, color, {0, 1, 0}}
    };
    UINT indices[] = {0,2,1,0,3,2};

    D3D11_BUFFER_DESC vbd={}; vbd.Usage=D3D11_USAGE_DEFAULT; vbd.ByteWidth=sizeof(vertices); vbd.BindFlags=D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vinit={vertices}; ID3D11Buffer* vbuf=nullptr; device->CreateBuffer(&vbd,&vinit,&vbuf);

    D3D11_BUFFER_DESC ibd={}; ibd.Usage=D3D11_USAGE_DEFAULT; ibd.ByteWidth=sizeof(indices); ibd.BindFlags=D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA iinit={indices}; ID3D11Buffer* ibuf=nullptr; device->CreateBuffer(&ibd,&iinit,&ibuf);

    RenderObject plane{};
    plane.vertexBuffer=vbuf; plane.indexBuffer=ibuf; plane.indexCount=ARRAYSIZE(indices); plane.position={0,0,0};
    return plane;
}

void Scene::Initialize(ID3D11Device* device)
{
    staticObjects.clear();
    dynamicObjects.clear();

    // Земля
    staticObjects.push_back(CreatePlane(device, 20.0f, {0.6f,0.6f,0.6f,1.0f}));

    // Рандомные объекты
    srand(static_cast<unsigned>(time(nullptr)));
    constexpr int numObjects=20;
    for(int i=0;i<numObjects;i++){
        const float size=0.1f+static_cast<float>(rand())/RAND_MAX*0.2f;
        DirectX::XMFLOAT4 color={
            static_cast<float>(rand())/RAND_MAX,
            static_cast<float>(rand())/RAND_MAX,
            static_cast<float>(rand())/RAND_MAX,
            1
        };
        float x=-10+static_cast<float>(rand())/RAND_MAX*20.0f;
        float z=-10+static_cast<float>(rand())/RAND_MAX*20.0f;
        float y=size/2.0f;

        RenderObject obj = (rand()%2==0)
            ? CreateCube(device,size,color)
            : CreateSphere(device,size/2,12,12,color);

        obj.position={x,y,z};
        dynamicObjects.push_back(obj);
    }

    // Игрок (куб)
    //player.sphere = CreateCube(device, 0.6f, {1, 0, 0, 1});
    player.sphere = CreateSphere(device, 0.3f, 12, 12, {1, 0, 0, 1});
    player.sphere.position = {0.0f, 0.3f, 0.0f};
    DirectX::XMStoreFloat4x4(&player.sphere.rotationMatrix, DirectX::XMMatrixIdentity());
}


void Scene::Update(float deltaTime, const InputHandler& input,
                   const DirectX::XMVECTOR& camForward, const DirectX::XMVECTOR& camRight)
{
    // --- Считываем ввод ---
    DirectX::XMFLOAT3 moveDir{0,0,0};
    if(input.IsKeyDown(Keys::W)) moveDir.z -= 1;
    if(input.IsKeyDown(Keys::S)) moveDir.z += 1;
    if(input.IsKeyDown(Keys::A)) moveDir.x -= 1;
    if(input.IsKeyDown(Keys::D)) moveDir.x += 1;

    DirectX::XMVECTOR dirVec = DirectX::XMVectorSet(moveDir.x, 0, moveDir.z, 0);
    if(DirectX::XMVector3Length(dirVec).m128_f32[0] > 0)
        dirVec = DirectX::XMVector3Normalize(dirVec);

    // --- Движение игрока относительно камеры ---
    DirectX::XMVECTOR moveVector = DirectX::XMVectorZero();
    moveVector = DirectX::XMVectorAdd(moveVector, DirectX::XMVectorScale(camForward, DirectX::XMVectorGetZ(dirVec)));
    moveVector = DirectX::XMVectorAdd(moveVector, DirectX::XMVectorScale(camRight,   DirectX::XMVectorGetX(dirVec)));
    moveVector = DirectX::XMVectorSetY(moveVector, 0);

    float len = DirectX::XMVector3Length(moveVector).m128_f32[0];
    DirectX::XMVECTOR moveDirNorm = (len > 0.001f) ? DirectX::XMVector3Normalize(moveVector) : DirectX::XMVectorZero();

    float speed = 5.0f;
    DirectX::XMFLOAT3 move{};
    XMStoreFloat3(&move, moveDirNorm);
    player.velocity.x = move.x * speed;
    player.velocity.z = move.z * speed;

    // --- Прыжок ---
    if(input.WasKeyPressedThisFrame(Keys::Space)){
        if(player.canJump || player.jumpCount < 2){
            player.velocity.y = 5.0f;
            player.jumpCount++;
            player.canJump = false;
        }
    }

    // --- Гравитация ---
    player.velocity.y += -9.81f * deltaTime;

    // --- Обновление позиции игрока ---
    player.sphere.position.x += player.velocity.x * deltaTime;
    player.sphere.position.y += player.velocity.y * deltaTime;
    player.sphere.position.z += player.velocity.z * deltaTime;

    if(player.sphere.position.y < player.sphere.radius){
        player.sphere.position.y = player.sphere.radius;
        player.velocity.y = 0;
        player.canJump = true;
        player.jumpCount = 0;
    }

    // --- Вращение шара по направлению движения ---
    DirectX::XMVECTOR horizontalMove = DirectX::XMVectorSet(player.velocity.x, 0, player.velocity.z, 0);
    float moveLen = DirectX::XMVector3Length(horizontalMove).m128_f32[0];

    if(moveLen > 0.001f) {
        horizontalMove = DirectX::XMVector3Normalize(horizontalMove);

        // локальная вертикаль игрока
        DirectX::XMVECTOR localUp = DirectX::XMVectorSet(0,1,0,0);

        // ось вращения для "катания" (рассчитывается в мировом пространстве)
        DirectX::XMVECTOR rotAxis = DirectX::XMVector3Cross(horizontalMove, localUp);
        rotAxis = DirectX::XMVector3Normalize(rotAxis);

        // угол вращения (инвертирован для корректного направления)
        float rotationAngle = -moveLen * deltaTime / player.sphere.radius;

        // создаём кватернион вращения
        DirectX::XMVECTOR rotQuat = DirectX::XMQuaternionRotationAxis(rotAxis, rotationAngle);

        // аккумулируем к существующей ориентации (используем rotationQuat)
        DirectX::XMVECTOR currentQuat = XMLoadFloat4(&player.sphere.rotationQuat);

        // ИСПРАВЛЕНО: Post-multiplication (current * rot) для корректного роллинга
        DirectX::XMVECTOR newQuat = DirectX::XMQuaternionMultiply(currentQuat, rotQuat);
        newQuat = DirectX::XMQuaternionNormalize(newQuat);

        // Сохраняем кватернион и обновляем матрицу вращения для рендера
        XMStoreFloat4(&player.sphere.rotationQuat, newQuat);
        XMStoreFloat4x4(&player.sphere.rotationMatrix, DirectX::XMMatrixRotationQuaternion(newQuat));
    }

    // --- Прилипание объектов ---
    for(int i=0; i<dynamicObjects.size(); i++){
        auto& obj = dynamicObjects[i];
        if(obj.attached) continue;

        float radiusPlayer = player.sphere.radius;
        float radiusObj = obj.radius;

        DirectX::XMVECTOR playerPos = XMLoadFloat3(&player.sphere.position);
        DirectX::XMVECTOR objPos = XMLoadFloat3(&obj.position);

        DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(objPos, playerPos);
        float distSq = DirectX::XMVector3LengthSq(diff).m128_f32[0];

        if(distSq <= (radiusPlayer + radiusObj)*(radiusPlayer + radiusObj)){
            obj.attached = true;

            // Мировое смещение (от игрока до объекта)
            DirectX::XMVECTOR worldOffset = diff;

            // Нам нужно найти локальное смещение, трансформируя мировое
            // WorldOffset = PlayerRot * LocalOffset => LocalOffset = PlayerRot_Inv * WorldOffset

            // 1. Получаем обратную матрицу вращения игрока
            DirectX::XMMATRIX playerRot = XMLoadFloat4x4(&player.sphere.rotationMatrix);
            DirectX::XMMATRIX playerRotInv = DirectX::XMMatrixInverse(nullptr, playerRot);

            // 2. Трансформируем мировое смещение в локальное (умножаем на обратную матрицу)
            DirectX::XMVECTOR localOffsetVector = XMVector3TransformNormal(worldOffset, playerRotInv);

            DirectX::XMFLOAT3 localOffset;
            XMStoreFloat3(&localOffset, localOffsetVector);

            // player.attachedObjects.push_back({i, localDir, dist});
            player.attachedObjects.push_back({i, localOffset}); // Сохраняем полный локальный вектор смещения
        }
    }

    // --- Обновление позиций прилипших объектов ---
    DirectX::XMMATRIX playerRot = XMLoadFloat4x4(&player.sphere.rotationMatrix);
    DirectX::XMVECTOR playerPos = XMLoadFloat3(&player.sphere.position);

    for(auto& ao : player.attachedObjects){
        auto& obj = dynamicObjects[ao.index];
        // Загружаем сохраненный локальный вектор смещения
        DirectX::XMVECTOR localOffset = XMLoadFloat3(&ao.localOffset);

        // Трансформируем локальное смещение текущей матрицей вращения игрока в мировое
        DirectX::XMVECTOR worldOffset = XMVector3TransformNormal(localOffset, playerRot);

        // Мировая позиция объекта = Позиция игрока + Мировое смещение
        DirectX::XMVECTOR worldPos = DirectX::XMVectorAdd(playerPos, worldOffset);
        XMStoreFloat3(&obj.position, worldPos);

        // Обновляем матрицу вращения объекта, чтобы он вращался вместе с шаром
        XMStoreFloat4x4(&obj.rotationMatrix, playerRot);
    }
}
