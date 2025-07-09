//
// Created by kron2 on 19.06.2025.
//

#ifndef CAMERA_H
#define CAMERA_H

#include <DirectXMath.h>

class Camera {
    public:
        virtual ~Camera() = default;

        [[nodiscard]] virtual DirectX::XMMATRIX GetViewMatrix() const = 0;
        virtual void SetPosition(float x, float y, float z) = 0;
        virtual void SetRotation(float pitch, float yaw) = 0;
        virtual void MoveForward(float amount) = 0;
        virtual void MoveBackward(float amount) = 0;
        virtual void MoveRight(float amount) = 0;
        virtual void MoveLeft(float amount) = 0;
        virtual void MoveUp(float amount) = 0;
        virtual void MoveDown(float amount) = 0;
        virtual void Rotate(float dx, float dy) = 0;
        virtual void Zoom(float amount) = 0;
};



#endif //CAMERA_H
