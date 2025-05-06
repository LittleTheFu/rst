#ifndef CAMERA_H
#define CAMERA_H

#include <Eigen/Dense>
#include <corecrt_math_defines.h>

// 定义相机移动方向的枚举
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
    // 定义不同的相机移动速度
    float movementSpeed = 2.5f;
    float mouseSensitivity = 0.1f;
    float zoomSensitivity = 0.5f; // 用于滚轮缩放的灵敏度
    float fov = 45.0f;
    float aspectRatio = 800.0f / 600.0f; // 默认宽高比
    float nearClip = 0.1f;
    float farClip = 100.0f;

    // 构造函数
    Camera(const Eigen::Vector3f &position = Eigen::Vector3f(0.0f, 0.0f, 3.0f),
           const Eigen::Vector3f &worldUp = Eigen::Vector3f(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f,
           float pitch = 0.0f);

    // 获取观察矩阵 (世界空间到观察空间的变换)
    Eigen::Matrix4f GetViewMatrix() const;

    // 获取投影矩阵 (观察空间到裁剪空间的变换 - 透视投影)
    Eigen::Matrix4f GetProjectionMatrix() const;

    // 处理键盘输入
    void ProcessKeyboard(CameraMovement direction, float deltaTime);

    // 处理鼠标移动输入
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

    // 处理鼠标滚轮输入 (用于缩放，改变 FOV)
    void ProcessMouseScroll(float yoffset);

    // 设置宽高比
    void setAspectRatio(float width, float height);

    // 获取相机的位置
    Eigen::Vector3f getPosition() const;

    // 获取相机的朝向 (Front 向量)
    Eigen::Vector3f getFront() const;

private:
    // 相机的位置
    Eigen::Vector3f Position;
    // 世界空间中的上方向
    Eigen::Vector3f WorldUp;
    // 相机的朝向 (指向目标)
    Eigen::Vector3f Front;
    // 相机右方向
    Eigen::Vector3f Right;
    // 相机上方向 (垂直于 Front 和 Right)
    Eigen::Vector3f Up;
    // 偏航角 (绕世界空间上方向旋转)
    float Yaw;
    // 俯仰角 (绕相机右方向旋转)
    float Pitch;

    // 更新 Front, Right 和 Up 向量
    void updateCameraVectors();
};


#endif // CAMERA_H