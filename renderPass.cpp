#include "RenderPass.h"
#include <iostream>
// #include "SDL_video.h" // 通常不需要在 RenderPass.cpp 中包含 SDL 视频头文件

RenderPass::RenderPass(const std::string &name, int width, int height)
    : name_(name),
      width_(width),
      height_(height)
{
    // Framebuffer 成员在这里只是被声明，具体的创建和配置（包括附件）
    // 将在派生类中完成。
    // 可以在这里直接初始化一个空的 Framebuffer 实例，或者在派生类中赋值。
    // 为了灵活性，我们倾向于在派生类中完整地初始化它。
}

void RenderPass::Resize(int width, int height)
{
    // 如果 Pass 内部的 Framebuffer 尺寸需要改变，通常意味着它的附件也需要重新创建。
    // 这部分逻辑将由派生类负责，因为派生类拥有具体的附件。
    width_ = width;
    height_ = height;
    // 如果 frameBuffer_ 已经存在，可以更新其内部的尺寸信息，但不能直接 resize 附件。
    // if (frameBuffer_) { frameBuffer_->updateSize(width, height); } // 如果 Framebuffer 有这样的方法
}

// 移除 createFramebuffer，因为附件创建由派生类和 Texture 类负责
// void RenderPass::createFramebuffer(const std::vector<GLenum>& colorFormats, DepthStencilAttachmentType dsType)
// {
//     frameBuffer_ = std::make_unique<Framebuffer>( width_, height_, colorFormats, dsType);
// }

// 移除这些获取附件的方法，因为基类不拥有具体纹理，无法提供
// std::vector<GLuint> RenderPass::getColorAttachments() const { /* ... */ }
// GLuint RenderPass::getColorAttachment(size_t index) const { /* ... */ }
// GLuint RenderPass::getDepthAttachment() const { /* ... */ }

void RenderPass::activateFramebuffer() const
{
    if (frameBuffer_)
    {
        frameBuffer_->activate();
    } else {
        // 可以选择抛出异常或打印警告，如果 frameBuffer_ 未初始化就被调用
        std::cerr << "Warning: Attempted to activate uninitialized framebuffer for pass " << name_ << std::endl;
    }
}

void RenderPass::deactivateFramebuffer() const
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