#include "brightnessMaskPass.h"
#include "debug_utils.h"
#include "utilities.h"

BrightnessMaskPass::BrightnessMaskPass(int width, int height)
    : RenderPass("BrightnessMaskPass", width, height)
{
    shader_.load("shader/brightnessMask.vert", "shader/brightnessMask.frag");

    maskTexture_ = std::make_unique<Texture2D>(Utilities::generateUniqueTextureId(),width_, height_, GL_RGBA8);
    maskTexture_->setParameters();

    frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);
    frameBuffer_->attachColorTexture(maskTexture_->id(), GL_COLOR_ATTACHMENT0);
    frameBuffer_->checkCompleteness();
}

void BrightnessMaskPass::Render(GLuint inputTexture)
{
    activateFramebuffer();
    setViewport(width_, height_);
    clearBuffers(GL_COLOR_BUFFER_BIT);

    shader_.use();

    glBindTextureUnit(0, inputTexture);
    shader_.setInt("sceneTexture", 0);

    screenQuad_.render();

    deactivateFramebuffer();

    GL_CHECK_ERROR();
}

void BrightnessMaskPass::Resize(int width, int height)
{
    if (width == width_ && height == height_) return;
    RenderPass::Resize(width, height);

    deactivateFramebuffer();

    maskTexture_ = std::make_unique<Texture2D>(Utilities::generateUniqueTextureId(),width_, height_, GL_RGBA8);
    maskTexture_->setParameters();

    frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);
    frameBuffer_->attachColorTexture(maskTexture_->id(), GL_COLOR_ATTACHMENT0);
    frameBuffer_->checkCompleteness();
}

GLuint BrightnessMaskPass::getOutputTextureId() const
{
    return maskTexture_->id();
}
