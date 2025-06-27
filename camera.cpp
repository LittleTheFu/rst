#include "camera.h"
#include <Eigen/Dense>
// #include <corecrt_math_defines.h> // 如果 M_PI 已在 camera.h 或其他全局头文件中定义，这里无需再次包含
#include <cmath> // 确保包含 cmath 以使用 std::asin, std::atan2
#include <iostream>

Camera::Camera(const Eigen::Vector3f &position, const Eigen::Vector3f &worldUp, float yaw, float pitch)
    : Position(position), WorldUp(worldUp), Yaw(yaw), Pitch(pitch)
{
    // 构造函数中初始化相机向量，确保Front, Right, Up在创建时就是正确的
    updateCameraVectors();
}

Eigen::Matrix4f Camera::GetViewMatrix() const
{
    // 构建视图矩阵
    // 通常通过 lookAt(eye, center, up) 来实现
    // 其中 center = eye + front
    // 这里我们直接根据已计算的 Front, Right, Up 向量来构建
    // 视图矩阵是将世界坐标系转换到相机坐标系的矩阵
    // 它由相机本地坐标系的基向量（Right, Up, -Front）以及相机位置的平移组成

    // 相机Z轴基向量 (指向观察目标的反方向)
    Eigen::Vector3f zaxis = -Front; 
    // 相机X轴基向量 (Right向量)
    Eigen::Vector3f xaxis = Right; // 直接使用已经计算好的 Right 向量
    // 相机Y轴基向量 (Up向量)
    Eigen::Vector3f yaxis = Up;    // 直接使用已经计算好的 Up 向量

    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();
    // 旋转部分 (相机局部基向量的转置)
    view(0, 0) = xaxis.x(); view(0, 1) = xaxis.y(); view(0, 2) = xaxis.z();
    view(1, 0) = yaxis.x(); view(1, 1) = yaxis.y(); view(1, 2) = yaxis.z();
    view(2, 0) = zaxis.x(); view(2, 1) = zaxis.y(); view(2, 2) = zaxis.z();
    
    // 平移部分 (相机位置的负点积)
    view(0, 3) = -xaxis.dot(Position);
    view(1, 3) = -yaxis.dot(Position);
    view(2, 3) = -zaxis.dot(Position); // 注意这里是 -zaxis.dot(Position)，因为zaxis是-Front
    return view;
}

Eigen::Matrix4f Camera::GetProjectionMatrix() const
{
    // 构建透视投影矩阵
    // fov * M_PI / 360.0f 等同于 fov / 2.0f 弧度
    float tanHalfFOV = std::tan(fov * M_PI / 360.0f);
    Eigen::Matrix4f projectionMatrix = Eigen::Matrix4f::Zero();
    projectionMatrix(0, 0) = 1.0f / (aspectRatio * tanHalfFOV);
    projectionMatrix(1, 1) = 1.0f / tanHalfFOV;
    projectionMatrix(2, 2) = -(farClip + nearClip) / (farClip - nearClip);
    projectionMatrix(2, 3) = -(2.0f * farClip * nearClip) / (farClip - nearClip);
    projectionMatrix(3, 2) = -1.0f;
    // projectionMatrix(3, 3) = 0.0f; // 这行是多余的，Zero() 已经设为0
    return projectionMatrix;
}

// 获取相机当前位置
Eigen::Vector3f Camera::getPosition() const
{
    return Position;
}

// 获取相机当前前方向量
Eigen::Vector3f Camera::getFront() const
{
    return Front;
}

void Camera::setFOV(float fov)
{
    this->fov = fov;
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
{
    // std::cout << "Position before: " << Position.transpose() << std::endl;

    // TODO: refactor later
    if (direction == ROT_LEFT || direction == ROT_RIGHT)
    {
        float velocity = rotationSpeed * deltaTime;
        if (direction == ROT_LEFT)
            Yaw -= velocity;
        if (direction == ROT_RIGHT)
            Yaw += velocity;
        updateCameraVectors();
        return;
    }
    
    float velocity = movementSpeed * deltaTime;
    if (direction == FORWARD)
        Position += Front * velocity;
    if (direction == BACKWARD)
        Position -= Front * velocity;
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;
    // 自由飞行相机模式：UP/DOWN 沿着相机局部Up向量移动
    if (direction == UP)
        Position += Up * velocity;
    if (direction == DOWN)
        Position -= Up * velocity;

    // std::cout << "Position after: " << Position.transpose() << std::endl;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    // 确保俯仰角不会超出限制 (通常是 -89 到 89 度)，避免万向锁问题
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
    fov -= yoffset * zoomSensitivity; // 使用 zoomSensitivity
    // 限制FOV范围，防止极端值
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 80.0f) // 将最大FOV调整到更合理的范围，例如 80度
        fov = 80.0f;
}

// 统一的 setAspectRatio 方法，只接收比例值
void Camera::setAspectRatio(float ratio)
{
    aspectRatio = ratio; 
}


void Camera::lookAt(const Eigen::Vector3f &target)
{
    Eigen::Vector3f direction = (target - Position).normalized();

    // 根据目标方向计算新的 Pitch 和 Yaw
    // 这里需要注意 atan2 的参数顺序以及 YAW 的初始值
    Pitch = std::asin(direction.y()) * 180.0f / M_PI; // 俯仰角基于Y分量
    // atan2(y, x) -> atan2(Z, X) for yaw, assuming initial yaw looks along -Z or +X
    // 假设 yaw=0 时相机朝向 (1,0,0) (正X轴)，那么：
    // newFront.x() = cos(Yaw) * cos(Pitch)
    // newFront.z() = sin(Yaw) * cos(Pitch)
    // 所以 Yaw = atan2(newFront.z(), newFront.x())
    Yaw = std::atan2(direction.z(), direction.x()) * 180.0f / M_PI;

    // 更新 Front, Right, Up 向量以反映新的视角
    updateCameraVectors();
}

void Camera::updateCameraVectors()
{
    // 根据当前的 Yaw 和 Pitch 重新计算 Front 向量
    Eigen::Vector3f front;
    // 欧拉角到方向向量的转换
    front.x() = std::cos(Yaw * M_PI / 180.0f) * std::cos(Pitch * M_PI / 180.0f);
    front.y() = std::sin(Pitch * M_PI / 180.0f);
    front.z() = std::sin(Yaw * M_PI / 180.0f) * std::cos(Pitch * M_PI / 180.0f);
    Front = front.normalized(); // 确保是单位向量

    // 根据 Front 和 WorldUp 计算 Right 向量 (注意叉乘顺序)
    // Right向量与WorldUp和Front都垂直
    Right = (Front.cross(WorldUp)).normalized();
    
    // 根据 Right 和 Front 计算 Up 向量 (确保它是正交的局部上向量)
    Up = (Right.cross(Front)).normalized(); // 注意叉乘顺序，保证 Right, Up, Front 构成右手坐标系
}