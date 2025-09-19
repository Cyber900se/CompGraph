#include "ThirdPersonCamera.h"

void ThirdPersonCamera::SetTarget(const DirectX::XMFLOAT3& pos)
{
    targetPos = pos;
}

DirectX::XMMATRIX ThirdPersonCamera::GetViewMatrix() const
{
    DirectX::XMFLOAT3 offset{
        distance * cosf(pitch) * sinf(yaw),
        distance * sinf(pitch),
        distance * cosf(pitch) * cosf(yaw)
    };
    DirectX::XMVECTOR eye = DirectX::XMVectorSet(
        targetPos.x + offset.x,
        targetPos.y + offset.y + 0.3f,
        targetPos.z + offset.z, 1.0f
    );
    DirectX::XMVECTOR focus = DirectX::XMLoadFloat3(&targetPos);
    DirectX::XMVECTOR upVec = DirectX::XMVectorSet(0,1,0,0);
    return DirectX::XMMatrixLookAtLH(eye, focus, upVec);
}

void ThirdPersonCamera::Rotate(float dx, float dy)
{
    yaw += dx;
    pitch += dy;
    pitch = std::clamp(pitch, 0.1f, 1.5f);
}

void ThirdPersonCamera::Update(const DirectX::XMFLOAT3& newTargetPos, float yawDelta, float pitchDelta)
{
    yaw += yawDelta;
    pitch += pitchDelta;
    pitch = std::clamp(pitch, -DirectX::XM_PIDIV2 + 0.1f, DirectX::XM_PIDIV2 - 0.1f);
    targetPos = newTargetPos;
    distance = 8.0f;
    float height = 3.0f;

    camPos.x = targetPos.x - distance * sinf(yaw) * cosf(pitch);
    camPos.y = targetPos.y + height + distance * sinf(pitch);
    camPos.z = targetPos.z - distance * cosf(yaw) * cosf(pitch);

    camTarget = targetPos;
    DirectX::XMStoreFloat4x4(&viewMatrix,
        DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&camPos),
                                  DirectX::XMLoadFloat3(&camTarget),
                                  DirectX::XMVectorSet(0,1,0,0))
    );
}


// Пустые методы для абстрактного интерфейса
void ThirdPersonCamera::SetPosition(float, float, float) {}
void ThirdPersonCamera::SetRotation(float, float) {}
void ThirdPersonCamera::MoveForward(float) {}
void ThirdPersonCamera::MoveBackward(float) {}
void ThirdPersonCamera::MoveRight(float) {}
void ThirdPersonCamera::MoveLeft(float) {}
void ThirdPersonCamera::MoveUp(float) {}
void ThirdPersonCamera::MoveDown(float) {}
void ThirdPersonCamera::Zoom(float) {}
