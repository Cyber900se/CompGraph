#ifndef CG_LAB4_THIRDPERSONCAMERA_H
#define CG_LAB4_THIRDPERSONCAMERA_H

#include <DirectXMath.h>
#include <algorithm>
#include "Camera.h"

class ThirdPersonCamera : public Camera
{
public:
    ThirdPersonCamera() = default;
    ~ThirdPersonCamera() override = default;

    // Основные свойства камеры
    DirectX::XMFLOAT3 camPos{};
    DirectX::XMFLOAT3 camTarget{};
    DirectX::XMFLOAT4X4 viewMatrix{};
    DirectX::XMFLOAT3 targetPos{};
    float distance = 5.0f;
    float pitch = 0.3f;
    float yaw = 0.0f;
    void SetTarget(const DirectX::XMFLOAT3& pos);

    [[nodiscard]] DirectX::XMMATRIX GetViewMatrix() const override;
    void Rotate(float dx, float dy) override;
    void Update(const DirectX::XMFLOAT3& targetPos, float yawDelta, float pitchDelta);

    DirectX::XMVECTOR GetPositionVector() override { return DirectX::XMLoadFloat3(&camPos); }

    // Пустые методы для соответствия абстрактному классу Camera
    void SetPosition(float, float, float) override;
    void SetRotation(float, float) override;
    void MoveForward(float) override;
    void MoveBackward(float) override;
    void MoveRight(float) override;
    void MoveLeft(float) override;
    void MoveUp(float) override;
    void MoveDown(float) override;
    void Zoom(float) override;
};

#endif // CG_LAB4_THIRDPERSONCAMERA_H
