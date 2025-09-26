#ifndef CG_LAB4_RENDEROBJECT_H
#define CG_LAB4_RENDEROBJECT_H

struct Vertex {
    DirectX::XMFLOAT4 position;
    DirectX::XMFLOAT4 color;
    DirectX::XMFLOAT3 normal; // ← для Phong
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
    int parentIndex = -1;
    DirectX::XMFLOAT4 color{};
    D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    bool attached = false;
    bool isOrbit = false;
    float radius = 0.15f;
    DirectX::XMFLOAT4X4 rotationMatrix = {};   // для совместимости с рендером
    DirectX::XMFLOAT4 rotationQuat = {0,0,0,1}; // добавляем кватернион для логики вращения
};
#endif //CG_LAB4_RENDEROBJECT_H