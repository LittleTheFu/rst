#ifndef TRANSFORMABLE_H
#define TRANSFORMABLE_H

#include <Eigen/Dense>

class Transformable {
public:
    virtual ~Transformable() = default;

    // 获取模型矩阵 (模型空间到世界空间的变换)
    virtual Eigen::Matrix4f getModelMatrix() const = 0;

    // 设置模型矩阵
    virtual void setModelMatrix(const Eigen::Matrix4f& modelMatrix) = 0;

    // 可选：提供更细粒度的控制
    virtual Eigen::Vector3f getPosition() const { return Eigen::Vector3f::Zero(); }
    virtual void setPosition(const Eigen::Vector3f& position) {}

    virtual Eigen::Quaternionf getRotation() const { return Eigen::Quaternionf::Identity(); }
    virtual void setRotation(const Eigen::Quaternionf& rotation) {}

    virtual Eigen::Vector3f getScale() const { return Eigen::Vector3f::Ones(); }
    virtual void setScale(const Eigen::Vector3f& scale) {}
};

#endif // TRANSFORMABLE_H