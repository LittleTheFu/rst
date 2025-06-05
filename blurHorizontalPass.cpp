#include "BlurHorizontalPass.h"
#include <iostream>
#include "debug_utils.h"
#include <assert.h>
#include <Eigen/Dense>

BlurHorizontalPass::BlurHorizontalPass(int width, int height)
    : RenderPass("BlurHorizontallPass", width, height),
      screenQuad_()
{
    shader_.load("shader/blurX.vert", "shader/blurX.frag");
    init();
}

void BlurHorizontalPass::Render(GLuint inputTexture)
{
    activateFramebuffer();
    setViewport(width_, height_);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader_.use();
    glBindTextureUnit(0, inputTexture);
    shader_.setInt("image", 0);
    shader_.setVec2("texelSize", Eigen::Vector2f(1.0f / width_, 1.0f / height_));

    screenQuad_.render();
    deactivateFramebuffer();

    GL_CHECK_ERROR();
}

void BlurHorizontalPass::Resize(int width, int height)
{
    RenderPass::Resize(width, height);
    setViewport(width_, height_);
    deactivateFramebuffer();
    init();
}

void BlurHorizontalPass::init()
{
    colorTexture_ = std::make_unique<Texture2D>(width_, height_, GL_RGBA32F);
    colorTexture_->setParameters();

    frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);
    frameBuffer_->attachColorTexture(colorTexture_->id(), GL_COLOR_ATTACHMENT0, 0);

    std::vector<GLenum> drawBuffers = {GL_COLOR_ATTACHMENT0};
    frameBuffer_->setDrawBuffers(drawBuffers);

    frameBuffer_->checkCompleteness();

    GL_CHECK_ERROR();
}

GLuint BlurHorizontalPass::getColorTextureId() const
{
    assert(colorTexture_ && "Color texture not initialized!");
    return colorTexture_->id();
}
