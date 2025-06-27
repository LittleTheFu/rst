#include "shadowPass.h"
#include <iostream>
#include <vector> 
#include "debug_utils.h"
#include "shadowUtils.h"
#include "sceneObject.h" // !!! 确保包含 ISceneObject 接口

// 辅助函数：初始化 ShadowPass 内部的 Framebuffer 和纹理
void ShadowPass::initializeFramebufferAndTextures()
{
    // 1. 创建 Framebuffer 对象
    frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);

    // 2. 创建一个立方体深度纹理（作为 Shadow Pass 的输出）
    shadowMapDepthTestTexture_ = std::make_unique<TextureCubeMap>(width_, GL_DEPTH_COMPONENT32F, 1); // 1 个 mip level
    shadowMapDepthTestTexture_->setParameters(); // 设置纹理参数

    shadowMapDepthOutputTexture_ = std::make_unique<TextureCubeMap>(width_, GL_R32F, 1); // 1 个 mip level
    shadowMapDepthOutputTexture_->setParameters(); // 设置纹理参数

    // 3. 激活 FBO，并告诉 OpenGL 不渲染到任何颜色附件
    frameBuffer_->activate();

    frameBuffer_->attachColorTexture(shadowMapDepthOutputTexture_->id(), GL_COLOR_ATTACHMENT0);
    frameBuffer_->attachDepthTexture(shadowMapDepthTestTexture_->id());

    std::vector<GLenum> drawBuffersVec = {GL_COLOR_ATTACHMENT0};
    frameBuffer_->setDrawBuffers(drawBuffersVec);

    // 4. 检查 FBO 的完整性
    frameBuffer_->checkCompleteness();
    frameBuffer_->deactivate();
}

ShadowPass::ShadowPass(int width, int height)
    : RenderPass("ShadowPass", width, height)
{
    shader_.load("shader/depth.vert", "shader/depth.frag");

    initializeFramebufferAndTextures();
}

// !!! 关键改动 !!!
// Render 方法现在接收 const std::vector<ISceneObject*>& 对象列表
void ShadowPass::Render(const std::vector<ISceneObject*>& objects, const PointLight &light)
{
    // 如果没有可用的阴影贴图或对象，直接返回
    if (!shadowMapDepthTestTexture_ || !shadowMapDepthOutputTexture_ || objects.empty())
    {
        return;
    }

    // 1. 绑定阴影 Pass 的 Framebuffer (立方体贴图的各个面)
    activateFramebuffer();
    setViewport(width_, height_);

    float farPlane = 100.0f;
    shader_.use();

    shader_.setVec3("lightPos", light.position); // 光源位置
    shader_.setFloat("farPlane", farPlane);      // 阴影贴图的远裁剪面距离

    std::vector<Eigen::Matrix4f> lightSpaceMatrices =
        ShadowUtils::CalculatePointLightSpaceMatrices(
            light.position,
            0.1f,       // 阴影贴图的近裁剪面
            farPlane,   // 阴影贴图的远裁剪面
            width_,     // 阴影贴图的宽度
            height_     // 阴影贴图的高度
        );

    // 3. 启用深度测试和裁剪
    enableState(GL_DEPTH_TEST);
    // glCullFace(GL_BACK); // 渲染阴影时通常剔除前面，以减少阴影痤疮

    for (int face = 0; face < 6; ++face)
    {
        // 绑定深度纹理的当前面作为深度附件
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 
                               shadowMapDepthTestTexture_->id(), 0); // 0是mip level

        // 绑定颜色纹理的当前面作为颜色附件
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                               shadowMapDepthOutputTexture_->id(), 0); // 0是mip level
        
        frameBuffer_->checkCompleteness();
        // 2. 清除深度缓冲
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


        shader_.setMat4("lightSpaceMatrix", lightSpaceMatrices[face]); // 设置当前面的视图投影矩阵
        
        // !!! 关键改动 !!!
        // 6. 渲染所有可投射阴影的 ISceneObject
        for (ISceneObject* obj : objects) // 遍历 ISceneObject*
        {
            if (obj == nullptr)
                continue; // 避免空指针

            // 每个 ISceneObject 都需要能提供自己的模型矩阵
            shader_.setMat4("model", obj->getModelMatrix()); 
            
            // !!! 关键改动 !!!
            // 调用 ISceneObject 的 draw 方法，并传入 shader
            // 确保 ISceneObject::draw(Shader&) 能够正确处理自己的渲染逻辑
            obj->render(shader_); 
        }
    }

    // 7. 恢复 OpenGL 状态
    // glCullFace(GL_FRONT); // 恢复背面剔除

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
    initializeFramebufferAndTextures();
}