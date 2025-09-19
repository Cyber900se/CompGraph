#ifndef FPSCAMERA_H
#define FPSCAMERA_H

#pragma once

#include "Camera.h"

#include <DirectXMath.h>
#include <algorithm>

class FPSCamera : public Camera {
public:
    FPSCamera();

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
    void Zoom(float amount) override {}  // FPS-камера не поддерживает Zoom

private:
    void UpdateVectors();

    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 front;
    DirectX::XMFLOAT3 up;
    DirectX::XMFLOAT3 right;

    float pitch;
    float yaw;
};

#endif // FPSCAMERA_H
