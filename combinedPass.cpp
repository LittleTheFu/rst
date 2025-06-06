#include "combinedPass.h"
#include <iostream>
#include "debug_utils.h"
#include <cassert>

CombinedPass::CombinedPass(int width, int height)
    : RenderPass("CombinedPass", width, height),
      screenQuad_()
{
    shader_.load("shader/combine.vert", "shader/combine.frag");
    init();
}

void CombinedPass::Render(GLuint directLightTextureID,
                          GLuint iblTextureID,
                          GLuint gpassDepthTextureID,
                          GLuint oitAccumTextureID,
                          GLuint oitRevealTextureID,
                          GLuint skyboxTextureID,
                          GLuint ssrTextureID)
{
    activateFramebuffer();

    setViewport(width_, height_);

    // 清除屏幕
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 禁用深度测试
    disableState(GL_DEPTH_TEST);

    // 绑定 Screen Shader
    shader_.use();

    // 绑定输入纹理
    glBindTextureUnit(0, directLightTextureID);
    shader_.setInt("directLightTexture", 0);

    glBindTextureUnit(1, iblTextureID);
    shader_.setInt("iblTexture", 1);

    glBindTextureUnit(2, gpassDepthTextureID);
    shader_.setInt("gpassDepthTexture", 2);

    glBindTextureUnit(3, oitAccumTextureID);
    shader_.setInt("accumTexture", 3);

    glBindTextureUnit(4, oitRevealTextureID);
    shader_.setInt("revealTexture", 4);

    glBindTextureUnit(5, skyboxTextureID);
    shader_.setInt("skyboxTexture", 5);

    glBindTextureUnit(6, ssrTextureID);
    shader_.setInt("ssrTexture", 6);

    screenQuad_.render(); 

    deactivateFramebuffer();

    GL_CHECK_ERROR();
}

void CombinedPass::Resize(int width, int height)
{
    // 更新基类的尺寸
    RenderPass::Resize(width, height);
    // 设置视口大小
    setViewport(width_, height_);

    init();
}

GLuint CombinedPass::getColorTextureId() const
{
    assert(colorTexture_);

    return colorTexture_->id();
}

void CombinedPass::init()
{
    colorTexture_ = std::make_unique<Texture2D>(width_, height_, GL_RGBA8); // 颜色
    colorTexture_->setParameters();

    // 创建深度纹理
    depthTexture_ = std::make_unique<Texture2D>(width_, height_, GL_DEPTH_COMPONENT24);

    // 创建 Framebuffer 对象
    frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);

    // 创建 G-Buffer 纹理附件
    frameBuffer_->attachColorTexture(colorTexture_->id(), GL_COLOR_ATTACHMENT0);

    // 创建深度纹理附件
    frameBuffer_->attachDepthTexture(depthTexture_->id(), 0);

    // 设置绘制缓冲区 (指定哪些颜色附件会被渲染)
    std::vector<GLenum> drawBuffers = {GL_COLOR_ATTACHMENT0};
    frameBuffer_->setDrawBuffers(drawBuffers);

    // 检查 Framebuffer 完整性
    frameBuffer_->checkCompleteness();

    GL_CHECK_ERROR();
}