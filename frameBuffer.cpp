#include "framebuffer.h"
#include <glad/glad.h>
#include <stdexcept>
#include <algorithm> // for std::max
#include <iostream>  // For debugging, if needed

// --- Constructor ---
Framebuffer::Framebuffer(int width, int height,
                         const std::vector<GLenum> &colorFormats,
                         DepthStencilAttachmentType dsType)
    : width_(width),
      height_(height),
      colorAttachmentFormats_(colorFormats), // 初始化颜色格式列表成员
      dsType_(dsType)                        // 初始化深度/模板类型成员
{
    // 参数基本验证
    if (width_ <= 0 || height_ <= 0)
    {
        THROW_GL_EXCEPTION("Framebuffer dimensions must be positive.");
    }

    
    GLint maxColorAttachments;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);
    if (static_cast<GLint>(colorAttachmentFormats_.size()) > maxColorAttachments)
    {
        THROW_GL_EXCEPTION("Number of color attachments exceeds GL_MAX_COLOR_ATTACHMENTS (" + std::to_string(maxColorAttachments) + ").");
    }

    glGenFramebuffers(1, &id_);
    if (id_ == 0)
    {
        THROW_GL_EXCEPTION("Failed to generate Framebuffer ID.");
    }

    // 将 FBO 绑定到 GL_FRAMEBUFFER 目标，所有后续的帧缓冲命令都会影响到这个 FBO
    // (理想情况下，这里会使用 FBOBindGuard 来保证异常安全解绑)
    glBindFramebuffer(GL_FRAMEBUFFER, id_);

    // 创建颜色附件
    createColorAttachments();

    // 创建深度/模板附件
    createDepthStencilAttachment();

    // 检查帧缓冲是否完整
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        THROW_GL_ERROR_EXCEPTION("Framebuffer is not complete!", status);
    }

    // 解绑帧缓冲，绑定回默认帧缓冲
    this->unbind();
}

// --- Destructor ---
Framebuffer::~Framebuffer()
{
    // 资源释放由 release() 负责，在 GLResource 析构函数中调用
}

// --- Bind ---
void Framebuffer::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, id_);
}

// --- Unbind ---
void Framebuffer::unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // 绑定到默认帧缓冲
}

// --- Create Color Attachments ---
void Framebuffer::createColorAttachments()
{
    // // 释放旧的颜色附件 (如果存在，通常在 resize 或构造函数内部重复调用时)
    for (GLuint textureID : colorAttachments_)
    {
        if (glIsTexture(textureID))
        { // 检查ID是否仍是有效的纹理，防止重复删除或删除0
            glDeleteTextures(1, &textureID);
        }
    }
    colorAttachments_.clear(); // 清空 vector，准备重新填充

    colorAttachments_.resize(colorAttachmentFormats_.size()); // 调整 vector 大小以容纳新的附件ID

    // 如果没有颜色附件，禁用颜色写入和读取
    if (colorAttachmentFormats_.empty())
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        return;
    }

    std::vector<GLenum> drawBuffers(colorAttachmentFormats_.size()); // 用于 glDrawBuffers 调用

    for (size_t i = 0; i < colorAttachmentFormats_.size(); ++i)
    {
        GLuint textureID = 0;
        glGenTextures(1, &textureID);
        if (textureID == 0)
        {
            THROW_GL_EXCEPTION("Failed to generate texture ID for color attachment " + std::to_string(i) + ".");
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        // 使用来自 formats 列表的对应格式
        // 对于大多数内部格式，GL_RGBA 和 GL_UNSIGNED_BYTE 的数据格式和类型是通用的，
        // 但对于某些特定格式（如浮点格式），可能需要调整。这里为了简化假设通用。
        glTexImage2D(GL_TEXTURE_2D, 0, colorAttachmentFormats_[i], width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0); // 解绑纹理

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textureID, 0);
        colorAttachments_[i] = textureID;
        drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
    }
    glDrawBuffers(static_cast<GLsizei>(colorAttachmentFormats_.size()), drawBuffers.data()); // 设置多重渲染目标
}

// --- Create Depth/Stencil Attachment ---
void Framebuffer::createDepthStencilAttachment()
{
    // 1. 先解绑旧的深度/模板附件 (防御性清理，确保不会残留旧附件)
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);

    // 2. 删除旧的附件ID (如果存在)
    if (depthStencilAttachment_ != 0)
    {
        // 根据存储的类型删除，避免删除错误类型的ID
        if (dsType_ == DepthStencilAttachmentType::Texture && glIsTexture(depthStencilAttachment_))
        {
            glDeleteTextures(1, &depthStencilAttachment_);
        }
        else if (dsType_ == DepthStencilAttachmentType::Renderbuffer && glIsRenderbuffer(depthStencilAttachment_))
        {
            glDeleteRenderbuffers(1, &depthStencilAttachment_);
        }
        depthStencilAttachment_ = 0; // 重置为0
    }

    // 3. 根据传入的 dsType 类型创建新的附件
    if (dsType_ == DepthStencilAttachmentType::Texture)
    {
        glGenTextures(1, &depthStencilAttachment_);
        if (depthStencilAttachment_ == 0)
        {
            THROW_GL_EXCEPTION("Failed to generate texture ID for depth/stencil attachment.");
        }
        glBindTexture(GL_TEXTURE_2D, depthStencilAttachment_);
        // 常见的深度模板格式
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width_, height_, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0); // 解绑纹理
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilAttachment_, 0);
    }
    else if (dsType_ == DepthStencilAttachmentType::Renderbuffer)
    {
        glGenRenderbuffers(1, &depthStencilAttachment_);
        if (depthStencilAttachment_ == 0)
        {
            THROW_GL_EXCEPTION("Failed to generate renderbuffer ID for depth/stencil attachment.");
        }
        glBindRenderbuffer(GL_RENDERBUFFER, depthStencilAttachment_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width_, height_);
        glBindRenderbuffer(GL_RENDERBUFFER, 0); // 解绑渲染缓冲区
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencilAttachment_);
    }
    // 如果 dsType 是 DepthStencilAttachmentType::None，则 depthStencilAttachment_ 保持为 0，不创建任何附件。
}

// --- Resize ---
void Framebuffer::resize(int width, int height)
{
    // 1. 输入验证
    if (width <= 0 || height <= 0)
    {
        THROW_GL_EXCEPTION("Framebuffer dimensions must be positive for resize.");
    }

    // 2. 如果尺寸未改变，则无需操作
    if (width_ == width && height_ == height)
    {
        return;
    }

    // 3. 更新成员变量
    width_ = width;
    height_ = height;

    // 4. 绑定 FBO，以便在其上进行操作
    this->bind(); // 使用类的 bind() 接口

    // 5. 重新创建颜色附件
    // createColorAttachments 内部会处理旧资源的释放和新资源的创建
    // 我们保存了 colorAttachmentFormats_，可以直接传入
    createColorAttachments();

    // 6. 重新创建深度/模板附件
    // createDepthStencilAttachment 内部会处理旧资源的释放和新资源的创建
    // 我们保存了 dsType_，可以直接传入
    createDepthStencilAttachment();

    // 7. 重新检查帧缓冲的完整性
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        THROW_GL_ERROR_EXCEPTION("Framebuffer is not complete after resize!", status);
    }

    // 8. 解绑 FBO
    this->unbind(); // 使用类的 unbind() 接口
}

// --- Release ---
void Framebuffer::release()
{
    // 确保 FBO 及其附件不会在被删除时处于绑定状态
    GLint currentFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
    if (currentFBO == (GLint)id_)
    { // 将 id_ 转换为 GLint 进行比较
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // 释放颜色附件纹理
    for (GLuint textureID : colorAttachments_)
    {
        if (glIsTexture(textureID))
        {
            glDeleteTextures(1, &textureID);
        }
    }
    colorAttachments_.clear(); // 清空 vector

    // 释放深度/模板附件
    if (depthStencilAttachment_ != 0)
    {
        if (dsType_ == DepthStencilAttachmentType::Texture && glIsTexture(depthStencilAttachment_))
        {
            glDeleteTextures(1, &depthStencilAttachment_);
        }
        else if (dsType_ == DepthStencilAttachmentType::Renderbuffer && glIsRenderbuffer(depthStencilAttachment_))
        {
            glDeleteRenderbuffers(1, &depthStencilAttachment_);
        }
    }
    depthStencilAttachment_ = 0; // 重置为0

    // 释放 FBO 本身
    if (id_ != 0)
    {
        glDeleteFramebuffers(1, &id_);
        id_ = 0;
    }
}