#include "FPSCamera.h"
#include <algorithm>

FPSCamera::FPSCamera() {
    position = {0.0f, 0.0f, -5.0f};
    pitch = 0.0f;
    yaw = 0.0f;
    UpdateVectors();
}

void FPSCamera::UpdateVectors() {
    // Вычисляем направление взгляда
    front = {
        cosf(yaw) * cosf(pitch),
        sinf(pitch),
        sinf(yaw) * cosf(pitch)
    };

    // Нормализуем векторы
    DirectX::XMVECTOR f = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&front));
    DirectX::XMStoreFloat3(&front, f);

    // Вычисляем правый и верхний векторы
    DirectX::XMVECTOR upVec = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR r = DirectX::XMVector3Cross(upVec, f);
    r = DirectX::XMVector3Normalize(r);
    DirectX::XMStoreFloat3(&right, r);

    DirectX::XMVECTOR u = DirectX::XMVector3Cross(f, r);
    u = DirectX::XMVector3Normalize(u);
    DirectX::XMStoreFloat3(&up, u);
}

DirectX::XMMATRIX FPSCamera::GetViewMatrix() const {
    DirectX::XMVECTOR eye = DirectX::XMLoadFloat3(&position);
    DirectX::XMVECTOR focus = DirectX::XMVectorAdd(eye, DirectX::XMLoadFloat3(&front));
    DirectX::XMVECTOR upVec = DirectX::XMLoadFloat3(&up);
    return DirectX::XMMatrixLookAtLH(eye, focus, upVec);
}

void FPSCamera::SetPosition(float x, float y, float z) {
    position = {x, y, z};
    UpdateVectors();
}

void FPSCamera::SetRotation(float pitch, float yaw) {
    this->pitch = pitch;
    this->yaw = yaw;
    UpdateVectors();
}

void FPSCamera::MoveBackward(float amount) {
    MoveForward(-amount);
}

void FPSCamera::MoveLeft(float amount) {
    MoveRight(-amount);
}

void FPSCamera::MoveForward(float amount) {
    DirectX::XMVECTOR move = DirectX::XMVectorScale(DirectX::XMLoadFloat3(&front), amount);
    DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&position);
    pos = DirectX::XMVectorAdd(pos, move);
    DirectX::XMStoreFloat3(&position, pos);
}

void FPSCamera::MoveRight(float amount) {
    DirectX::XMVECTOR move = DirectX::XMVectorScale(DirectX::XMLoadFloat3(&right), amount);
    DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&position);
    pos = DirectX::XMVectorAdd(pos, move);
    DirectX::XMStoreFloat3(&position, pos);
}

void FPSCamera::Rotate(float dx, float dy) {
    yaw += dx;
    pitch += dy;

    // Ограничиваем угол обзора по вертикали
    const float maxPitch = DirectX::XM_PIDIV2 - 0.01f;
    pitch = std::clamp(pitch, -maxPitch, maxPitch);

    UpdateVectors();
}

void FPSCamera::MoveUp(float amount) {
    DirectX::XMVECTOR move = DirectX::XMVectorSet(0.0f, amount, 0.0f, 0.0f);
    DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&position);
    pos = DirectX::XMVectorAdd(pos, move);
    DirectX::XMStoreFloat3(&position, pos);
}

void FPSCamera::MoveDown(float amount) {
    MoveUp(-amount);
}