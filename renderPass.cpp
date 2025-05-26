#include "RenderPass.h"
#include <iostream>
#include "SDL_video.h"

RenderPass::RenderPass(const std::string &name, int width, int height)
    : name_(name),
      width_(width),
      height_(height),
      frameBuffer_(nullptr)
{
}

// void RenderPass::Initialize()
// {+
// }

void RenderPass::Resize(int width, int height)
{
    // 默认实现可以为空，派生类可以根据需要重写
}

void RenderPass::createFramebuffer(const std::vector<GLenum>& colorFormats, DepthStencilAttachmentType dsType)
{
    frameBuffer_ = std::make_unique<Framebuffer>( width_, height_, colorFormats, dsType);
}

// GLuint RenderPass::createColorAttachment(int width, int height, GLenum internalFormat, GLenum format, GLenum type, GLenum attachment)
// {
//     GLuint texture;
//     glCreateTextures(GL_TEXTURE_2D, 1, &texture);
//     glTextureStorage2D(texture, 1, internalFormat, width, height);
//     glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//     glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//     glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture, 0);
//     colorAttachments_.push_back(texture);
//     return texture;
// }

std::vector<GLuint> RenderPass::getColorAttachments() const
{
    if(frameBuffer_)
    {
        return frameBuffer_->getColorAttachments();
    }

    return {};
}

GLuint RenderPass::getColorAttachment(size_t index) const
{
    if (frameBuffer_)
    {
        return frameBuffer_->getColorAttachment(index);
    }

    return 0;
}

GLuint RenderPass::getDepthAttachment() const
{
    if (frameBuffer_)
    {
        return frameBuffer_->getDepthStencilAttachment();
    }

    return 0;
}

void RenderPass::bindFramebuffer(bool read, bool draw)
{
    if (frameBuffer_)
    {
        frameBuffer_->activate();
    }
}

void RenderPass::unbindFramebuffer()
{
    if (frameBuffer_)
    {
        frameBuffer_->deactivate();
    }
}

void RenderPass::clearBuffers(GLbitfield mask)
{
    glClear(mask);
}

void RenderPass::setViewport(int width, int height)
{
    glViewport(0, 0, width, height);
}

void RenderPass::enableState(GLenum state)
{
    glEnable(state);
}

void RenderPass::disableState(GLenum state)
{
    glDisable(state);
}