#include "Camera.h"

Camera::Camera()
{
    this->fov = 45.0f;
    this->aspect = 8.0f / 6.0f;
    this->nearClip = 0.1f;
    this->farClip = 100.0f;
}

Camera::Camera(const Eigen::Vector3f &pos, const Eigen::Vector3f &target, const Eigen::Vector3f &up,
               float fov, float aspect, float nearClip, float farClip)
    : position(pos),
      viewDirection((target - pos).normalized()),
      upDirection(up.normalized()),
      target(target),
      fov(fov),
      aspect(aspect),
      nearClip(nearClip),
      farClip(farClip) {}

Eigen::Matrix4f Camera::getViewMatrix() const
{
    // 创建坐标系
    Eigen::Vector3f zAxis = -viewDirection;                        // 相机朝向的反方向
    Eigen::Vector3f xAxis = upDirection.cross(zAxis).normalized(); // 右方向
    Eigen::Vector3f yAxis = zAxis.cross(xAxis);                    // 上方向

    // 构造视图矩阵
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();
    view(0, 0) = xAxis.x();
    view(1, 0) = xAxis.y();
    view(2, 0) = xAxis.z(); // Right

    view(0, 1) = yAxis.x();
    view(1, 1) = yAxis.y();
    view(2, 1) = yAxis.z(); // Up

    view(0, 2) = zAxis.x();
    view(1, 2) = zAxis.y();
    view(2, 2) = zAxis.z(); // Forward

    // 视点平移
    view(0, 3) = -position.dot(xAxis);
    view(1, 3) = -position.dot(yAxis);
    view(2, 3) = -position.dot(zAxis);

    return view;
}

Eigen::Matrix4f Camera::getProjectionMatrix() const
{
    const float M_PI = 3.14159265358979323846f;
    float tanHalfFovy = tan(fov / 2.0f * M_PI / 180.0f); // 视野角度的一半的正切值

    Eigen::Matrix4f projection = Eigen::Matrix4f::Zero();
    projection(0, 0) = 1.0f / (aspect * tanHalfFovy); // 水平视野
    projection(1, 1) = 1.0f / tanHalfFovy;            // 垂直视野
    projection(2, 2) = -(farClip + nearClip) / (farClip - nearClip);
    projection(2, 3) = -(2.0f * farClip * nearClip) / (farClip - nearClip);
    projection(3, 2) = -1.0f;

    return projection;
}

void Camera::setPosition(const Eigen::Vector3f &newPos)
{
    position = newPos;
}

void Camera::setTarget(const Eigen::Vector3f &target)
{
    viewDirection = (target - position).normalized();
    this->target = target;
}

void Camera::setUpDirection(const Eigen::Vector3f &upDirection)
{
    this->upDirection = upDirection;
}

void Camera::setFOV(float newFov)
{
    fov = newFov;
}

void Camera::setClippingPlanes(float near, float far)
{
    nearClip = near;
    farClip = far;
}

void Camera::setAspect(float newAspect)
{
    aspect = newAspect;
}

void Camera::printCameraInfo() const
{
    std::cout << "Position: " << position.transpose() << std::endl;
    std::cout << "View Direction: " << viewDirection.transpose() << std::endl;
    std::cout << "Up Direction: " << upDirection.transpose() << std::endl;
}
