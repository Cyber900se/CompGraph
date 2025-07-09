#include "OrbitCamera.h"

#include <algorithm>

OrbitCamera::OrbitCamera() {
    target = {0.0f, 0.0f, 0.0f};
    distance = 5.0f;
    pitch = DirectX::XM_PIDIV4;
    yaw = DirectX::XM_PIDIV2;
    UpdatePosition();
}

void OrbitCamera::UpdatePosition() {
    position = {
        target.x + distance * cosf(yaw) * cosf(pitch),
        target.y + distance * sinf(pitch),
        target.z + distance * sinf(yaw) * cosf(pitch)
    };
}

DirectX::XMMATRIX OrbitCamera::GetViewMatrix() const {
    DirectX::XMVECTOR eye = DirectX::XMLoadFloat3(&position);
    DirectX::XMVECTOR focus = DirectX::XMLoadFloat3(&target);
    DirectX::XMVECTOR upVec = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    return DirectX::XMMatrixLookAtLH(eye, focus, upVec);
}

void OrbitCamera::SetPosition(float x, float y, float z) {
    target = {x, y, z};
    UpdatePosition();
}

void OrbitCamera::SetRotation(float pitch, float yaw) {
    this->pitch = pitch;
    this->yaw = yaw;
    UpdatePosition();
}

void OrbitCamera::Rotate(float dx, float dy) {
    yaw += dx;
    pitch += dy;

    // Ограничиваем угол обзора по вертикали
    const float maxPitch = DirectX::XM_PIDIV2 - 0.01f;
    pitch = std::clamp(pitch, -maxPitch, maxPitch);

    UpdatePosition();
}

void OrbitCamera::Zoom(float amount) {
    distance -= amount;
    distance = std::clamp(distance, 1.0f, 20.0f);
    UpdatePosition();
}