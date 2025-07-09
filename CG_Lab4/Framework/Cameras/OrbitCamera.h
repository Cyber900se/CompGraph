//
// Created by kron2 on 19.06.2025.
//

#ifndef ORBITCAMERA_H
#define ORBITCAMERA_H

#include "DirectXMath.h"
#include "Camera.h"

class OrbitCamera : public Camera {
    public:
        OrbitCamera();
        [[nodiscard]] DirectX::XMMATRIX GetViewMatrix() const override;
        void SetPosition(float x, float y, float z) override;
        void SetRotation(float pitch, float yaw) override;

        void Rotate(float dx, float dy) override;
        void Zoom(float amount) override;

        void MoveForward(float amount) override {}
        void MoveBackward(float amount) override {}
        void MoveRight(float amount) override {}
        void MoveLeft(float amount) override {}
        void MoveUp(float amount) override {}
        void MoveDown(float amount) override {}

    private:
        DirectX::XMFLOAT3 target{};
        float distance;
        float pitch;
        float yaw;
        DirectX::XMFLOAT3 position{};
        void UpdatePosition();
};

#endif //ORBITCAMERA_H
