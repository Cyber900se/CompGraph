#include "Scene.h"

RenderObject Scene::CreateCube(ID3D11Device* device, float size, const DirectX::XMFLOAT4& color,
                               float orbitRadius, float rotationSpeed, float orbitSpeed,
                               int parentIndex, float yOffset)
{
    const float half = size / 2.0f;
    Vertex vertices[] = {
        {{-half,-half,-half,1}, color}, {{-half, half,-half,1}, color}, {{ half, half,-half,1}, color}, {{ half,-half,-half,1}, color},
        {{-half,-half, half,1}, color}, {{-half, half, half,1}, color}, {{ half, half, half,1}, color}, {{ half,-half, half,1}, color},
    };

    UINT indices[] = {
        0,1,2, 0,2,3,
        4,6,5, 4,7,6,
        4,5,1, 4,1,0,
        3,2,6, 3,6,7,
        1,5,6, 1,6,2,
        4,0,3, 4,3,7
    };

    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = sizeof(vertices);
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vinit = {};
    vinit.pSysMem = vertices;
    ID3D11Buffer* vbuf = nullptr;
    device->CreateBuffer(&vbd, &vinit, &vbuf);

    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = sizeof(indices);
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA iinit = {};
    iinit.pSysMem = indices;
    ID3D11Buffer* ibuf = nullptr;
    device->CreateBuffer(&ibd, &iinit, &ibuf);

    RenderObject obj = {};
    obj.vertexBuffer = vbuf;
    obj.indexBuffer = ibuf;
    obj.indexCount = ARRAYSIZE(indices);
    obj.rotationAxis = {0,1,0};
    obj.rotationAngle = 0.0f;
    obj.rotationSpeed = rotationSpeed;
    obj.orbitRadius = orbitRadius;
    obj.orbitAngle = 0.0f;
    obj.orbitSpeed = orbitSpeed;
    obj.parentIndex = parentIndex;
    obj.orbitYOffset = yOffset;

    if (parentIndex >= 0 && parentIndex < static_cast<int>(objects.size())) {
        auto& p = objects[parentIndex];
        obj.position = { p.position.x + orbitRadius, p.position.y + yOffset, p.position.z };
    } else {
        obj.position = { orbitRadius, yOffset, 0.0f };
    }

    return obj;
}

RenderObject Scene::CreateSphere(ID3D11Device* device, float radius, int sliceCount, int stackCount,
                              const DirectX::XMFLOAT4& color, float orbitRadius, float rotationSpeed,
                              float orbitSpeed, int parentIndex, float yOffset)
{
    std::vector<Vertex> vertices;
    std::vector<UINT> indices;

    vertices.push_back({ {0, radius, 0, 1}, color });

    for(int i = 1; i < stackCount; ++i) {
        float phi = DirectX::XM_PI * i / stackCount;
        for(int j = 0; j < sliceCount; ++j) {
            float theta = 2.0f * DirectX::XM_PI * j / sliceCount;
            float x = radius * sinf(phi) * cosf(theta);
            float y = radius * cosf(phi);
            float z = radius * sinf(phi) * sinf(theta);
            vertices.push_back({ {x, y, z, 1}, color });
        }
    }

    vertices.push_back({ {0, -radius, 0, 1}, color });

    int northPole = 0;
    int southPole = static_cast<int>(vertices.size()) - 1;
    int ringVertexCount = sliceCount;

    for(int i = 0; i < sliceCount; ++i) {
        indices.push_back(northPole);
        indices.push_back(1 + i);
        indices.push_back(1 + (i + 1) % sliceCount);
    }

    int baseIndex = 1;
    int ringCount = stackCount - 2;
    for(int i = 0; i < ringCount - 1; ++i) {
        for(int j = 0; j < sliceCount; ++j) {
            int first  = baseIndex + i * ringVertexCount + j;
            int second = baseIndex + (i + 1) * ringVertexCount + j;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(baseIndex + i * ringVertexCount + (j + 1) % sliceCount);

            indices.push_back(second);
            indices.push_back(baseIndex + (i + 1) * ringVertexCount + (j + 1) % sliceCount);
            indices.push_back(baseIndex + i * ringVertexCount + (j + 1) % sliceCount);
        }
    }

    baseIndex = southPole - ringVertexCount;
    for(int i = 0; i < sliceCount; ++i) {
        indices.push_back(southPole);
        indices.push_back(baseIndex + (i + 1) % sliceCount);
        indices.push_back(baseIndex + i);
    }

    D3D11_BUFFER_DESC vbd = {};
    vbd.ByteWidth = sizeof(Vertex) * static_cast<UINT>(vertices.size());
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vinit = {};
    vinit.pSysMem = vertices.data();

    ID3D11Buffer* vbuf = nullptr;
    device->CreateBuffer(&vbd, &vinit, &vbuf);

    D3D11_BUFFER_DESC ibd = {};
    ibd.ByteWidth = sizeof(UINT) * static_cast<UINT>(indices.size());
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA iinit = {};
    iinit.pSysMem = indices.data();

    ID3D11Buffer* ibuf = nullptr;
    device->CreateBuffer(&ibd, &iinit, &ibuf);

    RenderObject obj = {};
    obj.vertexBuffer = vbuf;
    obj.indexBuffer = ibuf;
    obj.indexCount = static_cast<UINT>(indices.size());
    obj.rotationAxis = {0,1,0};
    obj.rotationAngle = 0.0f;
    obj.rotationSpeed = rotationSpeed;
    obj.orbitRadius = orbitRadius;
    obj.orbitAngle = 0.0f;
    obj.orbitSpeed = orbitSpeed;
    obj.parentIndex = parentIndex;
    obj.orbitYOffset = yOffset;

    if (parentIndex >= 0 && parentIndex < static_cast<int>(objects.size())) {
        auto& p = objects[parentIndex];
        obj.position = { p.position.x + orbitRadius, p.position.y + yOffset, p.position.z };
    } else {
        obj.position = { orbitRadius, yOffset, 0.0f };
    }

    return obj;
}

RenderObject Scene::CreateOrbit(ID3D11Device* device, int childIndex, const DirectX::XMFLOAT4& color)
{
    constexpr int segments = 64;
    std::vector<Vertex> vertices;
    std::vector<UINT> indices;

    float radius = objects[childIndex].orbitRadius;
    int parentIndex = objects[childIndex].parentIndex;

    for (int i = 0; i <= segments; ++i)
    {
        float theta = (2.0f * DirectX::XM_PI * i) / segments;
        float x = radius * cosf(theta);
        float z = radius * sinf(theta);
        vertices.push_back({ {x, 0.0f, z, 1}, color });

        if (i > 0) {
            indices.push_back(i - 1);
            indices.push_back(i);
        }
    }

    // создаём буферы
    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = (UINT)(sizeof(Vertex) * vertices.size());
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vinit = { vertices.data() };
    ID3D11Buffer* vbuf = nullptr;
    device->CreateBuffer(&vbd, &vinit, &vbuf);

    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = (UINT)(sizeof(UINT) * indices.size());
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA iinit = { indices.data() };
    ID3D11Buffer* ibuf = nullptr;
    device->CreateBuffer(&ibd, &iinit, &ibuf);

    RenderObject orbit = {};
    orbit.vertexBuffer = vbuf;
    orbit.indexBuffer = ibuf;
    orbit.indexCount = (UINT)indices.size();
    orbit.isOrbit = true;

    orbit.parentIndex   = parentIndex;                       // тот же родитель, что и у ребёнка
    orbit.orbitYOffset  = objects[childIndex].orbitYOffset;  // чтобы совпасть по Y с телом

    if (parentIndex >= 0 && parentIndex < (int)objects.size()) {
        const auto& p = objects[parentIndex];
        orbit.position = { p.position.x, p.position.y + orbit.orbitYOffset, p.position.z };
    } else {
        orbit.position = { 0.0f, orbit.orbitYOffset, 0.0f };
    }

    return orbit;
}

void Scene::Initialize(ID3D11Device* device)
{
    objects.clear();

    //Central body ("Sun")
    objects.push_back(CreateCube(device, 0.25f, {1,0.5f,0,1}, 0.0f, 1.0f, 0.0f, -1, 0.0f ));
    //Revolving bodies ("Planets")
    objects.push_back(CreateCube(device, 0.12f, {0,1,0,1}, 0.5f, 2.0f, 1.0f, 0, 0.0f));
    objects.push_back(CreateCube(device, 0.12f, {0,0,1,1}, 1.5f, 1.5f, 0.8f, 0, 0.0f));
    objects.push_back(CreateSphere(device, 0.1f, 16,16, {1,1,0,1}, 1.0f, 1.0f, 0.5f, 0, 0.0f));
    objects.push_back(CreateSphere(device, 0.15f,16,16, {1,0,1,1}, 2.2f, 3.0f, 1.2f, 0, 0.0f));
    // "Satellites"
    objects.push_back(CreateCube(device, 0.08f, {0,1,1,1}, 0.3f, 7.5f, 3.5f, 1, 0.0f));
    objects.push_back(CreateCube(device, 0.03f, {1,1,1,1}, 0.2f, 7.5f, 3.5f, 2, 0.0f));
    objects.push_back(CreateSphere(device, 0.06f, 12,12, {1,0.5f,0.5f,1}, 0.3f, 2.0f, 5.0f, 2, 0.0f));
    objects.push_back(CreateSphere(device, 0.017f, 12,12, {1,0,0.3f,1}, 0.3f, 2.0f, 7.0f, 4, 0.0f));
    // орбиты
    objects.push_back(CreateOrbit(device, 1, {1,1,1,1}));
    objects.push_back(CreateOrbit(device, 2, {1,1,1,1}));
    objects.push_back(CreateOrbit(device, 3, {1,1,1,1}));
    objects.push_back(CreateOrbit(device, 4, {1,1,1,1}));
    objects.push_back(CreateOrbit(device, 5, {1,1,1,1}));
    objects.push_back(CreateOrbit(device, 6, {1,1,1,1}));
    objects.push_back(CreateOrbit(device, 7, {1,1,1,1}));
    objects.push_back(CreateOrbit(device, 8, {1,1,1,1}));
}

void Scene::Update(float deltaTime)
{
    for (auto& obj : objects) {
        obj.rotationAngle -= obj.rotationSpeed * deltaTime;

        if (obj.isOrbit) {
            if (obj.parentIndex >= 0 && obj.parentIndex < (int)objects.size()) {
                const auto& p = objects[obj.parentIndex];
                obj.position = { p.position.x, p.position.y + obj.orbitYOffset, p.position.z };
            } else {
                obj.position = { 0.0f, obj.orbitYOffset, 0.0f };
            }
            continue;
        }
        if (obj.parentIndex >= 0 && obj.parentIndex < (int)objects.size()) {
            const RenderObject& parent = objects[obj.parentIndex];
            obj.orbitAngle += obj.orbitSpeed * deltaTime;
            float x = parent.position.x + obj.orbitRadius * cosf(obj.orbitAngle);
            float z = parent.position.z + obj.orbitRadius * sinf(obj.orbitAngle);
            float y = parent.position.y + obj.orbitYOffset;
            obj.position = { x, y, z };
        }
    }
}
