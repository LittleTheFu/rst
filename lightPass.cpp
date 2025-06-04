#include "lightPass.h"
#include <iostream>
#include <vector> // 确保包含 vector
#include "pointLightDataForUBO.h" // 确保包含 PointLightDataForUBO
#include "debug_utils.h"

LightPass::LightPass(int width, int height)
    : RenderPass("LightPass", width, height),screenQuad_()
{
    shader_.load("shader/light.vert", "shader/light.frag");

    // 激活 LightPass 的 Framebuffer
    activateFramebuffer();

    // 创建并附加 LightPass 的输出颜色纹理
    outputTexture_ = std::make_unique<Texture2D>(width_, height_, GL_RGBA32F); // RGBA8 作为颜色输出
    frameBuffer_->attachColorTexture(outputTexture_->id(), GL_COLOR_ATTACHMENT0);

    debugCurrentDepthTexture_ = std::make_unique<Texture2D>(width_, height_, GL_R32F);
    frameBuffer_->attachColorTexture(debugCurrentDepthTexture_->id(), GL_COLOR_ATTACHMENT1);

    debugClosestDepthTexture_ = std::make_unique<Texture2D>(width_, height_, GL_R32F);
    frameBuffer_->attachColorTexture(debugClosestDepthTexture_->id(), GL_COLOR_ATTACHMENT2);

    // 告诉 OpenGL 我们要渲染到哪些颜色附件
    std::vector<GLenum> drawBuffers = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    frameBuffer_->setDrawBuffers(drawBuffers);

    // 检查 Framebuffer 是否完整
    frameBuffer_->checkCompleteness();

    // 解激活 Framebuffer
    deactivateFramebuffer();

    // 初始化 Uniform Buffer Object
    lightBindingPoint_ = shader_.getUniformBlockIndex("PointLightBlock");
    objectLightUBO_.create(sizeof(PointLightDataForUBO), GL_DYNAMIC_DRAW);
    objectLightUBO_.bindToBindingPoint(lightBindingPoint_);
}

LightPass::~LightPass() {
    // 确保在析构时释放所有资源
}

void LightPass::Render(GLuint positionTextureID,
                       GLuint normalTextureID,
                       GLuint albedoTextureID,
                       GLuint roughnessTextureID,
                       GLuint metallicTextureID,
                       GLuint aoTextureID,
                       const PointLight &light,
                       const Camera &camera,
                       GLuint shadowMapID)
{
    // 1. 绑定 Light Pass 的 Framebuffer
    activateFramebuffer();
    setViewport(width_, height_);

    // 2. 清除颜色缓冲（深度缓冲通常由 G-Buffer 决定或不在此清除）
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    clearBuffers(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 3. 禁用深度测试（因为我们已经有了 G-Buffer 的深度，并且在这里渲染全屏四边形）
    disableState(GL_DEPTH_TEST);

    // 4. 绑定 Light Shader
    shader_.use();

    glBindTextureUnit(0, positionTextureID);
    shader_.setInt("positionTexture", 0);

    glBindTextureUnit(1, normalTextureID);
    shader_.setInt("normalTexture", 1);

    glBindTextureUnit(2, albedoTextureID);
    shader_.setInt("albedoTexture", 2);

    glBindTextureUnit(3, roughnessTextureID);
    shader_.setInt("roughnessTexture", 3);

    glBindTextureUnit(4, metallicTextureID);
    shader_.setInt("metallicTexture", 4);

    glBindTextureUnit(5, aoTextureID);
    shader_.setInt("aoTexture", 5);

    glBindTextureUnit(6, shadowMapID);
    shader_.setInt("shadowMapTexture", 6);

    // 7. 设置 Uniform 变量
    shader_.setVec3("viewPos", camera.Position);
    shader_.setVec3("light.position", light.position);
    shader_.setVec3("light.color", light.color);
    shader_.setFloat("light.intensity", light.intensity);
    shader_.setFloat("shadowCameraFarClip", 100.0f); // 应该与 ShadowPass 中的 far_plane 一致

    PointLightDataForUBO lightData;
    lightData.position = light.position;
    lightData.color = light.color;
    lightData.intensity = light.intensity;

    objectLightUBO_.updateData(0, sizeof(PointLightDataForUBO), &lightData);

    // 8. 渲染全屏四边形
    // renderQuad(); // 假设你有一个 renderQuad() 辅助函数来绘制全屏四边形
    screenQuad_.render();

    // 9. 解绑纹理
    // ... 实际应用中可以省略解绑，因为下一个绘制命令会重新绑定 ...

    // 10. 解绑 Light Pass 的 Framebuffer
    deactivateFramebuffer();

    GL_CHECK_ERROR();
}

void LightPass::Resize(int width, int height) {
    // 调用基类 Resize 更新宽度和高度
    RenderPass::Resize(width, height);

    // 解绑当前的 FBO，确保它不是活跃的
    deactivateFramebuffer();

    // 重置 Framebuffer (会删除旧的 FBO ID 并生成新的)
    frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);
    activateFramebuffer(); // 激活新的 Framebuffer

    // 重新创建并附加所有纹理到新的 Framebuffer
    outputTexture_ = std::make_unique<Texture2D>(width_, height_, GL_RGBA8);
    frameBuffer_->attachColorTexture(outputTexture_->id(), GL_COLOR_ATTACHMENT0);

    // 重新创建调试纹理
    debugCurrentDepthTexture_ = std::make_unique<Texture2D>(width_, height_, GL_R32F);
    frameBuffer_->attachColorTexture(debugCurrentDepthTexture_->id(), GL_COLOR_ATTACHMENT1);
    debugClosestDepthTexture_ = std::make_unique<Texture2D>(width_, height_, GL_R32F);
    frameBuffer_->attachColorTexture(debugClosestDepthTexture_->id(), GL_COLOR_ATTACHMENT2);


    // 重新设置 glDrawBuffers
    std::vector<GLenum> drawBuffers = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    frameBuffer_->setDrawBuffers(drawBuffers);

    // 检查 Framebuffer 完整性
    frameBuffer_->checkCompleteness();

    deactivateFramebuffer(); // 解激活
}