#include "shadowPass.h"
#include <iostream>
#include <vector> // 确保包含 vector
#include "debug_utils.h"
#include "shadowUtils.h"

// 辅助函数：初始化 ShadowPass 内部的 Framebuffer 和纹理
// 这里只负责创建 FBO 和 CubeMap 纹理对象，不进行附件操作
// 附件操作将在 Render 循环中针对每个面动态进行
void ShadowPass::initializeFramebufferAndTextures()
{
    // 1. 创建 Framebuffer 对象
    frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);

    // 2. 创建一个立方体深度纹理（作为 Shadow Pass 的输出）
    // internalFormat 通常为 GL_DEPTH_COMPONENT 或 GL_DEPTH_COMPONENT24/32F
    shadowMapDepthTestTexture_ = std::make_unique<TextureCubeMap>(width_, GL_DEPTH_COMPONENT32F, 1); // 1 个 mip level
    shadowMapDepthOutputTexture_ = std::make_unique<TextureCubeMap>(width_, GL_R32F, 1); // 1 个 mip level

    // 3. 激活 FBO，并告诉 OpenGL 不渲染到任何颜色附件
    frameBuffer_->activate();

    frameBuffer_->attachColorTexture(shadowMapDepthOutputTexture_->id(), GL_COLOR_ATTACHMENT0);
    frameBuffer_->attachDepthTexture(shadowMapDepthTestTexture_->id());

    std::vector<GLenum> drawBuffersVec = {GL_COLOR_ATTACHMENT0};
    frameBuffer_->setDrawBuffers(drawBuffersVec);

    // 4. 检查 FBO 的完整性
    // 注意：一个只有深度附件（或者没有附件但GL_NONE被设置为draw buffer）的FBO是完整的
    frameBuffer_->checkCompleteness();
    frameBuffer_->deactivate();
}

ShadowPass::ShadowPass(int width, int height)
    : RenderPass("ShadowPass", width, height)
{
    shader_.load("shader/depth.vert", "shader/depth.frag");

    // 调用辅助函数初始化 Framebuffer 和纹理
    initializeFramebufferAndTextures();
}

// 析构函数保持默认，unique_ptr 会自动清理资源
// ShadowPass::~ShadowPass() = default;

void ShadowPass::Render(const std::vector<const Mesh *> &meshes, const PointLight &light)
{
    // 如果没有可用的阴影贴图或网格，直接返回
    if (!shadowMapDepthTestTexture_ || !shadowMapDepthOutputTexture_ || meshes.empty())
    {
        return;
    }

    // 1. 绑定阴影 Pass 的 Framebuffer (立方体贴图的各个面)
    activateFramebuffer();
    setViewport(width_, height_);

    float farPlane = 100.0f;
    shader_.use();

    shader_.setVec3("lightPos", light.position); // 光源位置
    shader_.setFloat("farPlane", farPlane);        // 阴影贴图的远裁剪面距离

    std::vector<Eigen::Matrix4f> lightSpaceMatrices =
        ShadowUtils::CalculatePointLightSpaceMatrices(
            light.position,
            0.1f,   // 阴影贴图的近裁剪面
            farPlane, // 阴影贴图的远裁剪面
            width_, // 阴影贴图的宽度
            height_ // 阴影贴图的高度
        );

        
    // 3. 启用深度测试和裁剪（确保只渲染正面，避免阴影痤疮）
    enableState(GL_DEPTH_TEST);
    glCullFace(GL_FRONT); // 渲染阴影时通常剔除前面，以减少阴影痤疮


    for (int face = 0; face < 6; ++face)
    {
        // 2. 清除深度缓冲
        // 对于立方体阴影贴图，需要为每个面渲染前清除
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


        shader_.setMat4("lightSpaceMatrix", lightSpaceMatrices[face]); // 设置当前面的视图投影矩阵
        
        // 6. 渲染所有可投射阴影的网格
        for (const auto &mesh : meshes)
        {
            if (mesh == nullptr)
                continue; // 避免空指针

            shader_.setMat4("model", mesh->getModelMatrix());
            mesh->render(shader_); // 绘制网格，只关心深度
        }
    }

    // 7. 恢复 OpenGL 状态
    glCullFace(GL_BACK); // 恢复背面剔除

    // 8. 解绑阴影 Pass 的 Framebuffer
    deactivateFramebuffer();

    GL_CHECK_ERROR();
}

void ShadowPass::Resize(int width, int height)
{
    // 调用基类 Resize 更新宽度和高度
    RenderPass::Resize(width, height);

    // 解绑当前的 FBO，确保它不是活跃的
    deactivateFramebuffer();

    // 重新创建 Framebuffer 和纹理
    // initializeFramebufferAndTextures 会处理 Framebuffer 和 TextureCubeMap 的重新创建
    // 并设置 GL_NONE 作为绘制目标
    initializeFramebufferAndTextures();
}