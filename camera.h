#ifndef CAMERA_H
#define CAMERA_H

#include <Eigen/Dense>
#include <corecrt_math_defines.h>

enum CameraMovement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera
{
public:
    float movementSpeed = 2.5f;
    float mouseSensitivity = 0.1f;
    float zoomSensitivity = 0.5f;
    float fov = 45.0f;
    float aspectRatio = 800.0f / 600.0f;
    float nearClip = 0.1f;
    float farClip = 50.0f;

    Camera(const Eigen::Vector3f &position = Eigen::Vector3f(0.0f, 0.0f, 3.0f),
           const Eigen::Vector3f &worldUp = Eigen::Vector3f(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f,
           float pitch = 0.0f);

    void lookAt(const Eigen::Vector3f &target);
    void setAspectRatio(float ration);

    Eigen::Matrix4f LookAtCube(const Eigen::Vector3f &eye, const Eigen::Vector3f &center, const Eigen::Vector3f &up);

    Eigen::Matrix4f GetViewMatrix() const;
    Eigen::Matrix4f GetProjectionMatrix() const;

    void setFOV(float fov);

    void ProcessKeyboard(CameraMovement direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void ProcessMouseScroll(float yoffset);

    void setAspectRatio(float width, float height);

    Eigen::Vector3f getPosition() const;
    Eigen::Vector3f getFront() const;

public:
    Eigen::Vector3f Position;
    Eigen::Vector3f WorldUp;

    Eigen::Vector3f Front;
    Eigen::Vector3f Right;
    Eigen::Vector3f Up;

    float Yaw;
    float Pitch;

    void updateCameraVectors();
};


#endif