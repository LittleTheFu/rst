#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "Shader.h" // 假设所有可渲染对象都需要 Shader

class Renderable {
public:
    virtual ~Renderable() = default;

    // 所有可渲染对象都应该能够被渲染
    virtual void render(Shader& shader) = 0;

    // // 可选：获取模型矩阵，用于世界空间变换
    // virtual Eigen::Matrix4f getModelMatrix() const { return Eigen::Matrix4f::Identity(); }

    // // 可选：设置模型矩阵
    // virtual void setModelMatrix(const Eigen::Matrix4f& modelMatrix) {}
};

#endif // RENDERABLE_H