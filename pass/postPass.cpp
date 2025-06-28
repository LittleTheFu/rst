#include "PostPass.h"
#include <iostream>
#include "debug_utils.h"
#include <assert.h>
#include "utilities.h"
#include "shaderManager.h"

PostPass::PostPass(int width, int height)
    : RenderPass("PostPass", width, height),
      screenQuad_()
{
    shader_ = ShaderManager::getInstance().loadShader("shader/post.vert", "shader/post.frag");
    init();
}

void PostPass::Render(GLuint colorTextureID)
{
    activateFramebuffer();
    setViewport(width_, height_);

    // 清除屏幕
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // 绑定 Screen Shader
    shader_->use();

    // 绑定输入纹理
    glBindTextureUnit(0, colorTextureID);
    shader_->setInt("colorTexture", 0);

    screenQuad_.render();

    deactivateFramebuffer();
    GL_CHECK_ERROR();
}

void PostPass::Resize(int width, int height)
{
    // 更新基类的尺寸
    RenderPass::Resize(width, height);
    setViewport(width_, height_);

    deactivateFramebuffer();
    init();
}

void PostPass::init()
{
    colorTexture_ = std::make_unique<Texture2D>(Utilities::generateUniqueTextureId(),width_, height_, GL_RGBA32F);
    colorTexture_->setParameters();

    frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);
    frameBuffer_->attachColorTexture(colorTexture_->id(), GL_COLOR_ATTACHMENT0, 0);

    // 设置绘制缓冲区
    std::vector<GLenum> drawBuffers = {GL_COLOR_ATTACHMENT0};
    frameBuffer_->setDrawBuffers(drawBuffers);

    frameBuffer_->checkCompleteness();

    GL_CHECK_ERROR();
}

GLuint PostPass::getColorTextureId() const
{
    assert(colorTexture_ && "Color texture not initialized!");
    return colorTexture_->id();
}