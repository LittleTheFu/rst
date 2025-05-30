#include "oitPass.h"
#include <iostream>
#include "debug_utils.h" // 确保包含调试工具

OitPass::OitPass(int width, int height)
    : RenderPass("OitPass", width, height)
{
    shader_.load("shader/oit.vert", "shader/oit.frag");
    init(); // 初始化 G-Buffer FBO 和纹理
}

void OitPass::Render(const std::vector<const Mesh *> &meshes, const Camera &camera)
{
    activateFramebuffer();
    setViewport(width_, height_);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    clearBuffers(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_CULL_FACE);

    enableState(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);

    glBlendFuncSeparatei(0, GL_ONE, GL_ONE, GL_ONE, GL_ONE);
    glBlendFuncSeparatei(1, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);

    shader_.use();

    shader_.setMat4("projection", camera.GetProjectionMatrix());
    shader_.setMat4("view", camera.GetViewMatrix());

    for (const auto &mesh : meshes)
    {
        if (mesh == nullptr)
            continue;

        shader_.setMat4("model", mesh->getModelMatrix());
        mesh->render(shader_); // 绘制网格
    }

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);

    deactivateFramebuffer();
    GL_CHECK_ERROR(); // 检查 OpenGL 错误
}

void OitPass::init()
{
    colorTexture_ = std::make_unique<Texture2D>(width_, height_, GL_RGBA8); // 颜色
    colorTexture_->setParameters();

    alphaTexture_ = std::make_unique<Texture2D>(width_, height_, GL_R8); // 颜色
    alphaTexture_->setParameters();

    // 创建深度纹理
    depthTexture_ = std::make_unique<Texture2D>(width_, height_, GL_DEPTH_COMPONENT24);

    // 创建 Framebuffer 对象
    frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);

    // 创建 G-Buffer 纹理附件
    frameBuffer_->attachColorTexture(colorTexture_->id(), GL_COLOR_ATTACHMENT0);
    frameBuffer_->attachColorTexture(alphaTexture_->id(), GL_COLOR_ATTACHMENT1);

    // 创建深度纹理附件
    frameBuffer_->attachDepthTexture(depthTexture_->id(), 0);

    // 设置绘制缓冲区 (指定哪些颜色附件会被渲染)
    std::vector<GLenum> drawBuffers = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    frameBuffer_->setDrawBuffers(drawBuffers);

    // 检查 Framebuffer 完整性
    frameBuffer_->checkCompleteness();

    GL_CHECK_ERROR(); // 检查 OpenGL 错误
}

// 获取深度纹理
GLuint OitPass::getDepthAttachment() const
{
    assert(depthTexture_ && "Depth texture is not initialized!");
    return depthTexture_->id();
}

GLint OitPass::getColorTextureId() const
{
    assert(colorTexture_ && "Position texture is not initialized!");
    return colorTexture_->id();
}

// 重写 Resize 方法以重新创建 G-Buffer 纹理
void OitPass::Resize(int width, int height)
{
    // 如果尺寸没有变化，则无需重新创建
    if (width == width_ && height == height_)
    {
        return;
    }

    // 更新基类的尺寸
    RenderPass::Resize(width, height);

    // 首先解绑 FBO
    deactivateFramebuffer();

    // 重新创建 FBO 和附件
    init();

    GL_CHECK_ERROR(); // 检查 OpenGL 错误
}