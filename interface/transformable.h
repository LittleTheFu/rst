#ifndef TRANSFORMABLE_H
#define TRANSFORMABLE_H

#include <Eigen/Dense>

class ITransformable
{
public:
    virtual ~ITransformable() = default;

    virtual Eigen::Matrix4f getModelMatrix() const = 0;

    virtual void setModelMatrix(const Eigen::Matrix4f &modelMatrix) = 0;

    virtual Eigen::Vector3f getPosition() const { return Eigen::Vector3f::Zero(); }
    virtual void setPosition(const Eigen::Vector3f &position) {}

    virtual Eigen::Quaternionf getRotation() const { return Eigen::Quaternionf::Identity(); }
    virtual void setRotation(const Eigen::Quaternionf &rotation) {}

    virtual Eigen::Vector3f getScale() const { return Eigen::Vector3f::Ones(); }
    virtual void setScale(const Eigen::Vector3f &scale) {}
};

#endif