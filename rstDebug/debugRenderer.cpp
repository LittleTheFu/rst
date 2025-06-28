#include "debugRenderer.h"
#include "debug_utils.h"    // Your GL_CHECK_ERROR() macro
#include <Eigen/Geometry>   // For Eigen::Translation3f, Eigen::Scaling
#include <iostream>
#include <shaderManager.h>

// DebugRenderer 构造函数
DebugRenderer::DebugRenderer() : debugBox_() {
    // DebugBox 在这里自动初始化其 VAO/VBO/EBO
    // 视图和投影矩阵在 SetMatrices 中设置
}

// 初始化着色器
void DebugRenderer::InitShader(const std::string& vertexPath, const std::string& fragmentPath) {
    shader_ = ShaderManager::getInstance().loadShader(vertexPath.c_str(), fragmentPath.c_str());
    GL_CHECK_ERROR();
    if (!shader_ || !shader_->isValid()) {
        std::cerr << "ERROR: DebugRenderer shader initialization failed!" << std::endl;
    }
}

// 设置视图和投影矩阵
void DebugRenderer::SetMatrices(const Eigen::Matrix4f& view, const Eigen::Matrix4f& projection) {
    viewMatrix_ = view;
    projectionMatrix_ = projection;
}

void DebugRenderer::SaveGLState() const {
    glGetIntegerv(GL_POLYGON_MODE, originalPolygonMode_);
    originalCullFaceEnabled_ = glIsEnabled(GL_CULL_FACE);
    originalDepthTestEnabled_ = glIsEnabled(GL_DEPTH_TEST);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &originalDepthMaskEnabled_); // 注意这里需要地址

    // 确保 GL_DEPTH_TEST 是开启的，如果它本来是关闭的，并且你想让调试几何体参与深度测试
    if (!originalDepthTestEnabled_) {
        glEnable(GL_DEPTH_TEST);
    }
    // 确保深度写入是开启的，如果你希望调试几何体的深度影响后续渲染
    if (!originalDepthMaskEnabled_) {
        glDepthMask(GL_TRUE);
    }

    GL_CHECK_ERROR(); // 检查保存状态时的错误
}

void DebugRenderer::RestoreGLState() const {
    glPolygonMode(GL_FRONT_AND_BACK, originalPolygonMode_[0]); // 恢复多边形模式

    if (originalCullFaceEnabled_) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }

    if (originalDepthTestEnabled_) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }

    glDepthMask(originalDepthMaskEnabled_); // 恢复深度写入
    GL_CHECK_ERROR(); // 检查恢复状态时的错误
}

// 绘制 AABB
void DebugRenderer::DrawAABB(const AABB& aabb, const Eigen::Vector3f& color) const {
    if (!shader_ || !shader_->isValid()) {
        std::cerr << "DebugRenderer shader not initialized or invalid. Cannot draw AABB." << std::endl;
        return;
    }

    SaveGLState(); // 保存当前 OpenGL 状态

    shader_->use();
    shader_->setMat4("view", viewMatrix_);
    shader_->setMat4("projection", projectionMatrix_);
    shader_->setVec3("debugColor", color);
    GL_CHECK_ERROR();

    // 计算 AABB 的模型矩阵
    Eigen::Vector3f center = aabb.GetCenter();
    Eigen::Vector3f size = aabb.GetSize();

    // 构建模型矩阵：先缩放，后平移
    Eigen::Affine3f modelTransform =
        Eigen::Translation3f(center) * Eigen::Scaling(size);
    
    shader_->setMat4("model", modelTransform.matrix());
    GL_CHECK_ERROR();

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // 绘制线框模式
    debugBox_.render(); // 渲染 DebugBox
    GL_CHECK_ERROR();

    RestoreGLState(); // 恢复之前保存的 OpenGL 状态
}

// 绘制点光源
void DebugRenderer::DrawPointLight(const PointLight& light, float size, const Eigen::Vector3f& color) const {
    if (!shader_ || !shader_->isValid()) {
        std::cerr << "DebugRenderer shader not initialized or invalid. Cannot draw PointLight." << std::endl;
        return;
    }

    SaveGLState(); // 保存当前 OpenGL 状态

    shader_->use();
    shader_->setMat4("view", viewMatrix_);
    shader_->setMat4("projection", projectionMatrix_);
    shader_->setVec3("debugColor", color);
    GL_CHECK_ERROR();

    // 计算点光源的模型矩阵 (平移到位置，并进行小尺寸缩放)
    Eigen::Affine3f modelTransform =
        Eigen::Translation3f(light.position) * Eigen::Scaling(size, size, size); // 固定尺寸的小球体/立方体
    
    shader_->setMat4("model", modelTransform.matrix());
    GL_CHECK_ERROR();

    // 对于光源，我们可能想画一个实心小球或实心立方体，或者一个线框表示
    // 这里我们绘制实心立方体，你可以改为 GL_LINE 绘制线框
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // 绘制实心模式

    // 禁用深度写入，以确保光源调试标记总是可见，不被场景遮挡（可选）
    glDepthMask(GL_FALSE);
    
    debugBox_.render(); // 渲染 DebugBox (作为小立方体)
    GL_CHECK_ERROR();

    glDepthMask(GL_TRUE); // 恢复深度写入

    RestoreGLState(); // 恢复之前保存的 OpenGL 状态
}