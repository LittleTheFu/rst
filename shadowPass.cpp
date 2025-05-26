#include "shadowPass.h"
#include <iostream>
#include <vector> // 确保包含 vector

// 辅助函数：初始化 ShadowPass 内部的 Framebuffer 和纹理
// 这里只负责创建 FBO 和 CubeMap 纹理对象，不进行附件操作
// 附件操作将在 Render 循环中针对每个面动态进行
void ShadowPass::initializeFramebufferAndTextures() {
    // 1. 创建 Framebuffer 对象
    frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);

    // 2. 创建一个立方体深度纹理（作为 Shadow Pass 的输出）
    // internalFormat 通常为 GL_DEPTH_COMPONENT 或 GL_DEPTH_COMPONENT24/32F
    shadowMapTexture_ = std::make_unique<TextureCubeMap>(width_, height_, GL_DEPTH_COMPONENT);

    // 3. 激活 FBO，并告诉 OpenGL 不渲染到任何颜色附件
    frameBuffer_->activate();
    GLenum drawBuffers[] = { GL_NONE }; // 重要：只渲染深度，不渲染颜色
    frameBuffer_->setDrawBuffers(std::vector<GLenum>(drawBuffers, drawBuffers + 1));

    // 4. 检查 FBO 的完整性
    // 注意：一个只有深度附件（或者没有附件但GL_NONE被设置为draw buffer）的FBO是完整的
    frameBuffer_->checkCompleteness();
    frameBuffer_->deactivate();
}


ShadowPass::ShadowPass(int width, int height, SceneData& sceneData, Camera& lightCamera)
    : RenderPass("ShadowPass", width, height),
      sceneData_(sceneData),
      lightCamera_(lightCamera)
{
    shader_.load("shader/depth.vert", "shader/depth.frag");

    // 调用辅助函数初始化 Framebuffer 和纹理
    initializeFramebufferAndTextures();
}

// 析构函数保持默认，unique_ptr 会自动清理资源
// ShadowPass::~ShadowPass() = default;

void ShadowPass::Render()
{
    // 绑定 ShadowPass 的 Framebuffer 作为渲染目标
    activateFramebuffer();

    // 启用深度测试，通常是 GL_LESS
    enableState(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE); // 确保深度写入是开启的

    // 设置视口为阴影贴图的尺寸
    setViewport(width_, height_);

    // 使用深度着色器
    shader_.use();

    // 获取光源位置和投影矩阵
    Eigen::Vector3f lightPos = lightCamera_.Position; // 假设 lightCamera_ 的 Position 就是光源位置
    Eigen::Matrix4f projectionMatrix = lightCamera_.GetProjectionMatrix(); // 获取光源的投影矩阵

    // 准备立方体六个面的视图方向
    Eigen::Vector3f targets[6] = {
        lightPos + Eigen::Vector3f(1, 0, 0),    // +X
        lightPos + Eigen::Vector3f(-1, 0, 0),   // -X
        lightPos + Eigen::Vector3f(0, 1, 0),    // +Y
        lightPos + Eigen::Vector3f(0, -1, 0),   // -Y
        lightPos + Eigen::Vector3f(0, 0, 1),    // +Z
        lightPos + Eigen::Vector3f(0, 0, -1)    // -Z
    };

    Eigen::Vector3f ups[6] = {
        Eigen::Vector3f(0, -1, 0), // +X
        Eigen::Vector3f(0, -1, 0), // -X
        Eigen::Vector3f(0, 0, 1),  // +Y
        Eigen::Vector3f(0, 0, -1), // -Y
        Eigen::Vector3f(0, -1, 0), // +Z
        Eigen::Vector3f(0, -1, 0)  // -Z
    };

    shader_.setFloat("farClip", lightCamera_.farClip); // 传递光源的远裁剪面
    shader_.setVec3("lightPos", lightPos);

    for (int face = 0; face < 6; ++face)
    {
        glClear(GL_DEPTH_BUFFER_BIT); // 清除当前面的深度缓冲

        // 关键一步：使用 Framebuffer 的新接口来附加当前面
        frameBuffer_->attachDepthCubeMapFace(shadowMapTexture_->id(), GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0);

        // 重新检查 FBO 完整性（此调用在生产代码中可能移除以提高性能，因为每次都检查开销较大）
        frameBuffer_->checkCompleteness();

        // 绑定光源相机的视图矩阵到当前面
        Eigen::Matrix4f viewMat = Camera::LookAt(lightPos, targets[face], ups[face]); // 假设 Camera 有一个静态 LookAt 方法
        Eigen::Matrix4f lightSpaceMatrix = projectionMatrix * viewMat;
        shader_.setMat4("lightSpaceMatrix", lightSpaceMatrix); // 或者直接传递 view/projection，取决于你的着色器

        // 渲染场景中的所有对象
        for (const auto &object : sceneData_.objects)
        {
            shader_.setMat4("model", object->getModelMatrix());
            object->render(shader_);
        }
    }

    // 恢复状态和解绑 Framebuffer
    disableState(GL_DEPTH_TEST); // 恢复深度测试状态
    glDepthMask(GL_FALSE);      // 恢复深度写入为禁用（除非后续 Pass 明确需要）
    deactivateFramebuffer();
}

void ShadowPass::Resize(int width, int height) {
    // 调用基类 Resize 更新宽度和高度
    RenderPass::Resize(width, height);

    // 解绑当前的 FBO，确保它不是活跃的
    deactivateFramebuffer();

    // 重新创建 Framebuffer 和纹理
    // initializeFramebufferAndTextures 会处理 Framebuffer 和 TextureCubeMap 的重新创建
    // 并设置 GL_NONE 作为绘制目标
    initializeFramebufferAndTextures();
}