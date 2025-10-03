#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include "../InputHandler.h"
#include "RenderObject.h"

#include "../Cameras/Camera.h"

struct AttachedObject {
    int index;
    //DirectX::XMFLOAT3 localOffsetDir; // нормализованное направление от центра игрока
    //float distance;                    // расстояние до поверхности
    DirectX::XMFLOAT3 localOffset;
};

struct Player {
    RenderObject sphere;
    DirectX::XMFLOAT3 velocity{};
    bool canJump = true;
    int jumpCount = 0;

    DirectX::XMFLOAT4 rotationQuat{0, 0, 0, 1}; // единичный кватернион
    std::vector<AttachedObject> attachedObjects; // объекты, которые прилипли
};

struct SceneCamera {

    DirectX::XMFLOAT3 camPos;
    DirectX::XMFLOAT3 camTarget;
    float yaw = 0.0f;
    float pitch = 0.0f;
    DirectX::XMFLOAT4X4 viewMatrix;
};

inline bool CheckCollision(const RenderObject& a, const RenderObject& b, float aRadius, float bRadius) {
    DirectX::XMVECTOR pa = XMLoadFloat3(&a.position);
    DirectX::XMVECTOR pb = XMLoadFloat3(&b.position);

    DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(pa, pb);
    float distSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(diff));
    float r = aRadius + bRadius;
    return distSq <= r * r;
}

class Scene {
public:
    std::vector<RenderObject> objects;
    Player player;
    SceneCamera camera;

    void Initialize(ID3D11Device* device);
    void Update(float deltaTime, const InputHandler& input,
                const DirectX::XMVECTOR& camForward,
                const DirectX::XMVECTOR& camRight);

    RenderObject CreateCube(ID3D11Device* device, float size, const DirectX::XMFLOAT4& color,
                            float orbitRadius=0, float rotationSpeed=0, int parentIndex=-1, float yOffset=0);

    RenderObject CreateSphere(ID3D11Device* device, float radius, int sliceCount, int stackCount,
                              const DirectX::XMFLOAT4& color, float orbitRadius=0, float rotationSpeed=0,
                              int parentIndex=-1, float yOffset=0);

    const std::vector<RenderObject>& GetStaticObjects() const { return staticObjects; }
    const std::vector<RenderObject>& GetDynamicObjects() const { return dynamicObjects; }

    static RenderObject CreatePlane(ID3D11Device* device, float size, const DirectX::XMFLOAT4 &color);

    const std::vector<RenderObject>& GetObjects() const { return objects; }
private:
    std::vector<RenderObject> staticObjects;
    std::vector<RenderObject> dynamicObjects;
};
