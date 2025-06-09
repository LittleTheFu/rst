#include "RenderPass.h"
#include <iostream>
#include <stdexcept> // 用于 std::runtime_error

RenderPass::RenderPass(const std::string &name, int width, int height)
    : name_(name),
      width_(width),
      height_(height)
{
    // 在基类构造函数中直接创建 Framebuffer 实例。
    // 此时它只是一个空的 Framebuffer 容器，没有附件。
    // 附件的创建和附加将在派生类的构造函数中完成。
    frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);
    if (!frameBuffer_) {
        // 理论上 unique_ptr 不会失败除非内存耗尽，但此处做个防御性检查
        throw std::runtime_error("Failed to create Framebuffer for RenderPass: " + name_);
    }
}

void RenderPass::Resize(int width, int height)
{
    // 更新 Pass 的尺寸
    width_ = width;
    height_ = height;
    // 重要：这里的 Framebuffer Resize 逻辑仍然需要由派生类来处理。
    // 因为它们拥有具体的 Texture 附件，而这些附件需要在尺寸改变时重新分配存储。
    // Framebuffer 类本身没有 Resize 方法来修改其内部附件。
    // 如果 Framebuffer 类支持 resize 方法，可以在这里调用 frameBuffer_->resize(width, height);
}

void RenderPass::activateFramebuffer() const
{
    // 由于 Framebuffer 肯定在构造函数中创建了，这里无需检查 nullptr
    frameBuffer_->activate();
}

void RenderPass::deactivateFramebuffer() const
{
    frameBuffer_->deactivate();
}

void RenderPass::clearBuffers(GLbitfield mask)
{
    glClear(mask);
}

void RenderPass::setViewport(int width, int height) const
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