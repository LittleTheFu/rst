#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

// 你的 ECS 核心头文件 (假设你有 Entity 和 ComponentManager 或类似结构)
// #include "ECS/System.h" // 假设你有基类 System
// #include "ECS/Components.h" // 包含 TransformComponent 和 ModelRenderComponent

#include <memory>
#include <vector>
#include <iostream>

#include <Eigen/Dense> // For Eigen::Matrix4f

// 你已经创建的管理器
#include "ModelManager.h"
#include "MeshManager.h"
#include "MaterialManager.h"

// 你的着色器类
#include "shader.h" // 你的 Shader 类头文件

// ECS 组件（需要包含在你实际的组件定义中）
#include "TransformComponent.h"
#include "ModelRenderComponent.h"

// 假设你有某种方式来获取实体和它们的组件，例如一个 ECS 核心实例
// class ECSCore {
// public:
//     // ... 获取实体的接口
//     template<typename T>
//     T* getComponent(EntityID entityId);
// };

class RenderSystem // 如果你有一个 System 基类，则继承它
{
public:
    // 构造函数，可以接收 ECS 核心或其他初始化参数
    RenderSystem(Shader& shader, Eigen::Matrix4f& viewMatrix, Eigen::Matrix4f& projectionMatrix);

    // 假设你的系统有一个 Update 或 Render 方法
    // 它会接收一个实体列表或迭代器，并访问它们的组件
    // 这里为了演示，我们假设你有一个 getEntitiesWithComponents 方法
    void render(const std::vector<std::pair<unsigned int, std::pair<TransformComponent*, ModelRenderComponent*>>>& renderEntities);

    // 设置通用的场景 Uniforms (例如光源信息)
    void setGlobalUniforms(const Eigen::Vector3f& viewPos, const Eigen::Vector3f& lightDir, const Eigen::Vector3f& lightColor);

private:
    Shader& m_shader; // 渲染系统使用的通用着色器
    Eigen::Matrix4f& m_viewMatrix;
    Eigen::Matrix4f& m_projectionMatrix;

    // 获取单例管理器的引用
    ModelManager& m_modelManager;
    MeshManager& m_meshManager;
    MaterialManager& m_materialManager;
};

#endif // RENDER_SYSTEM_H