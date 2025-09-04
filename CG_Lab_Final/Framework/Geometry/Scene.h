#ifndef SCENE_H
#define SCENE_H

#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

struct Vertex {
    DirectX::XMFLOAT4 position;
    DirectX::XMFLOAT4 color;
};

struct RenderObject {
    ID3D11Buffer* vertexBuffer = nullptr;
    ID3D11Buffer* indexBuffer = nullptr;
    UINT indexCount = 0;

    DirectX::XMFLOAT3 position = {0,0,0};

    DirectX::XMFLOAT3 rotationAxis = {0,1,0};
    float rotationAngle = 0.0f;
    float rotationSpeed = 0.0f;

    float orbitRadius = 0.0f;
    float orbitAngle = 0.0f;
    float orbitSpeed = 0.0f;
    float orbitYOffset = 0.0f;

    // заменяем указатель на индекс родителя (-1 = нет родителя)
    int parentIndex = -1;
};
class Scene {
public:
    void Initialize(ID3D11Device* device);
    void Update(float deltaTime);

    RenderObject CreateCube( ID3D11Device* device, float size, const DirectX::XMFLOAT4& color,
                             float orbitRadius, float rotationSpeed, float orbitSpeed,
                             int parentIndex, float yOffset );

    RenderObject CreateSphere(ID3D11Device* device, float radius, int sliceCount, int stackCount,
                              const DirectX::XMFLOAT4& color, float orbitRadius, float rotationSpeed,
                              float orbitSpeed, int parentIndex, float yOffset);

    const std::vector<RenderObject>& GetObjects() const { return objects; }
private:
    std::vector<RenderObject> objects;
};

#endif