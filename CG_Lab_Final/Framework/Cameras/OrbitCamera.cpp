#include "OrbitCamera.h"

OrbitCamera::OrbitCamera() {
    target   = {0.0f, 0.0f, 0.0f};
    distance = 5.0f;
    pitch    = DirectX::XM_PIDIV4;   // 45°
    yaw      = DirectX::XM_PIDIV2;   // 90°, смотрим вдоль +Z
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
    DirectX::XMVECTOR eye   = XMLoadFloat3(&position);
    DirectX::XMVECTOR focus = XMLoadFloat3(&target);
    DirectX::XMVECTOR upVec = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    return DirectX::XMMatrixLookAtLH(eye, focus, upVec);
}

void OrbitCamera::SetPosition(float x, float y, float z) {
    target = {x, y, z};
    UpdatePosition();
}

void OrbitCamera::SetRotation(float pitch, float yaw) {
    this->pitch = pitch;
    this->yaw   = yaw;
    UpdatePosition();
}

void OrbitCamera::SetTarget(float x, float y, float z) {
    target = {x, y, z};
    UpdatePosition();
}

void OrbitCamera::SetRadius(float r) {
    distance = std::max(0.1f, r);
    UpdatePosition();
}

// --- Движение цели (панорамирование) ---
void OrbitCamera::MoveForward(float amount) {
    DirectX::XMVECTOR eye   = XMLoadFloat3(&position);
    DirectX::XMVECTOR focus = XMLoadFloat3(&target);
    DirectX::XMVECTOR f     = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(focus, eye));
    focus          = DirectX::XMVectorAdd(focus, DirectX::XMVectorScale(f, amount));
    XMStoreFloat3(&target, focus);
    UpdatePosition();
}

void OrbitCamera::MoveBackward(float amount) { MoveForward(-amount); }

void OrbitCamera::MoveRight(float amount) {
    DirectX::XMVECTOR eye   = XMLoadFloat3(&position);
    DirectX::XMVECTOR focus = XMLoadFloat3(&target);
    DirectX::XMVECTOR f     = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(focus, eye));
    DirectX::XMVECTOR up    = DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f);
    DirectX::XMVECTOR r     = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(up, f));
    focus          = DirectX::XMVectorAdd(focus, DirectX::XMVectorScale(r, amount));
    XMStoreFloat3(&target, focus);
    UpdatePosition();
}

void OrbitCamera::MoveLeft(float amount) { MoveRight(-amount); }

void OrbitCamera::MoveUp(float amount) {
    DirectX::XMVECTOR focus = XMLoadFloat3(&target);
    DirectX::XMVECTOR up    = DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f);
    focus          = DirectX::XMVectorAdd(focus, DirectX::XMVectorScale(up, amount));
    XMStoreFloat3(&target, focus);
    UpdatePosition();
}

void OrbitCamera::MoveDown(float amount) { MoveUp(-amount); }

// --- Вращение вокруг цели ---
void OrbitCamera::Rotate(float dx, float dy) {
    yaw   += dx;
    pitch += dy;

    const float maxPitch = DirectX::XM_PIDIV2 - 0.01f;
    pitch = std::clamp(pitch, -maxPitch, maxPitch);

    UpdatePosition();
}

// --- Зум (меняем расстояние до цели) ---
void OrbitCamera::Zoom(float amount) {
    distance -= amount;
    distance = std::clamp(distance, 1.0f, 20.0f);
    UpdatePosition();
}
