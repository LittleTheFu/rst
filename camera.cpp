#include "camera.h"
#include <Eigen/Dense>
#include <corecrt_math_defines.h>

Camera::Camera(const Eigen::Vector3f &position, const Eigen::Vector3f &worldUp, float yaw, float pitch)
    : Position(position), WorldUp(worldUp), Yaw(yaw), Pitch(pitch)
{
    updateCameraVectors();
}

Eigen::Matrix4f Camera::GetViewMatrix() const
{
    Eigen::Vector3f zaxis = (Position - (Position + Front)).normalized(); // camera direction
    Eigen::Vector3f xaxis = WorldUp.cross(zaxis).normalized();            // camera right
    Eigen::Vector3f yaxis = zaxis.cross(xaxis);                           // camera up

    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();
    view(0, 0) = xaxis.x(); view(0, 1) = xaxis.y(); view(0, 2) = xaxis.z(); view(0, 3) = -xaxis.dot(Position);
    view(1, 0) = yaxis.x(); view(1, 1) = yaxis.y(); view(1, 2) = yaxis.z(); view(1, 3) = -yaxis.dot(Position);
    view(2, 0) = zaxis.x(); view(2, 1) = zaxis.y(); view(2, 2) = zaxis.z(); view(2, 3) = -zaxis.dot(Position);
    return view;
}

Eigen::Matrix4f Camera::GetProjectionMatrix() const
{
    float tanHalfFOV = tan(fov * M_PI / 360.0f);
    Eigen::Matrix4f projectionMatrix = Eigen::Matrix4f::Zero();
    projectionMatrix(0, 0) = 1.0f / (aspectRatio * tanHalfFOV);
    projectionMatrix(1, 1) = 1.0f / tanHalfFOV;
    projectionMatrix(2, 2) = -(farClip + nearClip) / (farClip - nearClip);
    projectionMatrix(2, 3) = -(2.0f * farClip * nearClip) / (farClip - nearClip);
    projectionMatrix(3, 2) = -1.0f;
    projectionMatrix(3, 3) = 0.0f;
    return projectionMatrix;
}

void Camera::setFOV(float fov)
{
    this->fov = fov;
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
{
    float velocity = movementSpeed * deltaTime;
    if (direction == FORWARD)
        Position += Front * velocity;
    if (direction == BACKWARD)
        Position -= Front * velocity;
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;
    if (direction == UP)
        Position += Up * velocity;
    if (direction == DOWN)
        Position -= Up * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    // 确保俯仰角不会超出限制，避免万向锁
    if (constrainPitch)
    {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    // 更新 Front, Right 和 Up 向量
    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    fov -= (float)yoffset * zoomSensitivity;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}

void Camera::setAspectRatio(float width, float height)
{
    aspectRatio = width / height;
}

Eigen::Vector3f Camera::getPosition() const
{
    return Position;
}

Eigen::Vector3f Camera::getFront() const
{
    return Front;
}

void Camera::setAspectRatio(float ration)
{
    aspectRatio = ration; 
}


void Camera::lookAt(const Eigen::Vector3f &target)
{
    Eigen::Vector3f direction = (target - Position).normalized();

    Pitch = std::asin(direction.y()) * 180.0f / M_PI;
    Yaw = std::atan2(direction.z(), direction.x()) * 180.0f / M_PI;

    updateCameraVectors();
}

#include <vector>

Eigen::Matrix4f Camera::LookAtCube(const Eigen::Vector3f &eye, const Eigen::Vector3f &center, const Eigen::Vector3f &up)
{
    Eigen::Vector3f f = (center - eye).normalized();
    Eigen::Vector3f s = f.cross(up).normalized();
    Eigen::Vector3f u = s.cross(f);

    Eigen::Matrix4f mat = Eigen::Matrix4f::Identity();
    mat(0, 0) = s.x();
    mat(0, 1) = s.y();
    mat(0, 2) = s.z();
    mat(1, 0) = u.x();
    mat(1, 1) = u.y();
    mat(1, 2) = u.z();
    mat(2, 0) = -f.x();
    mat(2, 1) = -f.y();
    mat(2, 2) = -f.z();
    mat(0, 3) = -s.dot(eye);
    mat(1, 3) = -u.dot(eye);
    mat(2, 3) = -f.dot(eye);
    return mat;
}



void Camera::updateCameraVectors()
{
    Eigen::Vector3f front;
    front.x() = cos(Yaw * M_PI / 180.0f) * cos(Pitch * M_PI / 180.0f);
    front.y() = sin(Pitch * M_PI / 180.0f);
    front.z() = sin(Yaw * M_PI / 180.0f) * cos(Pitch * M_PI / 180.0f);
    Front = front.normalized();

    Right = (Front.cross(WorldUp)).normalized();
    
    Up = (Right.cross(Front)).normalized();
}