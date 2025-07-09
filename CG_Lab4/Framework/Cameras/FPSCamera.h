#ifndef FPSCAMERA_H
#define FPSCAMERA_H

#include "DirectXMath.h"
#include "Camera.h"

class FPSCamera : public Camera {
    public:
        FPSCamera();
        [[nodiscard]] DirectX::XMMATRIX GetViewMatrix() const override;
        void SetPosition(float x, float y, float z) override;
        void SetRotation(float pitch, float yaw) override;

        void MoveBackward(float amount) override;
        void MoveLeft(float amount) override;
        void MoveForward(float amount) override;
        void MoveRight(float amount) override;
        void MoveUp(float amount) override;
        void MoveDown(float amount) override;
        void Rotate(float dx, float dy) override;

        void Zoom(float amount) override {}

    private:
        DirectX::XMFLOAT3 position{};
        float pitch; // Вращение вверх/вниз (радианы)
        float yaw;   // Вращение влево/вправо (радианы)
        DirectX::XMFLOAT3 front{};
        DirectX::XMFLOAT3 right{};
        DirectX::XMFLOAT3 up{};
        void UpdateVectors();
};

#endif //FPSCAMERA_H
