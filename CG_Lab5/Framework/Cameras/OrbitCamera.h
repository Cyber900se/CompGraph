#ifndef ORBITCAMERA_H
#define ORBITCAMERA_H

#pragma once

#include "Camera.h"

#include <DirectXMath.h>
#include <algorithm>

class OrbitCamera : public Camera {
public:
    OrbitCamera();

    // Переопределения виртуальных методов Camera
    DirectX::XMMATRIX GetViewMatrix() const override;

    void SetPosition(float x, float y, float z) override;
    void SetRotation(float pitch, float yaw) override;

    void MoveForward(float amount) override;
    void MoveBackward(float amount) override;
    void MoveRight(float amount) override;
    void MoveLeft(float amount) override;
    void MoveUp(float amount) override;
    void MoveDown(float amount) override;

    void Rotate(float dx, float dy) override;
    void Zoom(float amount) override;

    // Дополнительно для OrbitCamera
    void SetTarget(float x, float y, float z);
    void SetRadius(float r);
    void UpdatePosition();

private:
    DirectX::XMFLOAT3 position{}; // где находится камера
    DirectX::XMFLOAT3 target{};   // на что смотрим
    float distance;             // расстояние до цели
    float pitch;                // угол вверх/вниз
    float yaw;                  // угол влево/вправо
};

#endif