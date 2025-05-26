#include "Framebuffer.h"
#include <glad/glad.h>
#include <stdexcept>
#include <algorithm> // for std::max
#include <iostream>  // For debugging, if needed
#include "debug_utils.h"

// --- Constructor ---
Framebuffer::Framebuffer(int width, int height,
                         const std::vector<GLenum> &colorFormats,
                         DepthStencilAttachmentType dsType)
    : width_(width),
      height_(height),
      colorAttachmentFormats_(colorFormats), // 初始化颜色格式列表成员
      dsType_(dsType)                         // 初始化深度/模板类型成员
{
    // 参数基本验证
    if (width_ <= 0 || height_ <= 0)
    {
        THROW_GL_EXCEPTION("Framebuffer dimensions must be positive.");
    }

    // 1. 使用 DSA 方式创建 Framebuffer 对象
    glCreateFramebuffers(1, &id_); // 直接创建 FBO，ID 存储在基类的 id_ 中
    if (id_ == 0)
    {
        THROW_GL_EXCEPTION("Failed to create Framebuffer ID (DSA).");
    }

    // 2. 检查最大颜色附件数量 (这部分与DSA无关，保持不变)
    GLint maxColorAttachments;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);
    if (static_cast<GLint>(colorAttachmentFormats_.size()) > maxColorAttachments)
    {
    THROW_GL_EXCEPTION("Number of color attachments exceeds GL_MAX_COLOR_ATTACHMENTS (" + std::to_string(maxColorAttachments) + ").");
    }

    // --- 创建附件（全部使用 DSA 方式）---
    createColorAttachments();
    GL_CHECK_ERROR();
    
    createDepthStencilAttachment();
    GL_CHECK_ERROR();

    // 检查帧缓冲是否完整 (DSA: glCheckNamedFramebufferStatus)
    GLenum status = glCheckNamedFramebufferStatus(id_, GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        THROW_GL_ERROR_EXCEPTION("Framebuffer is not complete!", status);
    }

    // 构造函数结束时，FBO 处于未绑定状态，这是 DSA 的自然结果。
}

// --- Bind (用于将FBO设置为渲染目标) ---
void Framebuffer::activate() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, id_);
}

// --- Unbind (用于将渲染目标切换回默认FBO) ---
void Framebuffer::deactivate() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // 绑定到默认帧缓冲
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

    // 4. 释放旧的颜色和深度/模板附件 (DSA: 附件删除不依赖 FBO 绑定状态)
    releaseAttachments();

    // 5. 重新创建颜色附件 (DSA)
    createColorAttachments();

    // 6. 重新创建深度/模板附件 (DSA)
    createDepthStencilAttachment();

    // 7. 重新检查帧缓冲的完整性 (DSA)
    GLenum status = glCheckNamedFramebufferStatus(id_, GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        THROW_GL_ERROR_EXCEPTION("Framebuffer is not complete after resize!", status);
    }
}

// --- Create Color Attachments (完全 DSA 化) ---
void Framebuffer::createColorAttachments()
{
    colorAttachments_.resize(colorAttachmentFormats_.size());

    // 如果没有颜色附件，禁用颜色写入和读取（这些是全局状态，非 DSA）
    if (colorAttachmentFormats_.empty())
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        return;
    }

    std::vector<GLenum> drawBuffers(colorAttachmentFormats_.size());

    for (size_t i = 0; i < colorAttachmentFormats_.size(); ++i)
    {
        GLuint textureID = 0;
        // 1. 使用 DSA 方式创建纹理
        glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
        if (textureID == 0)
        {
            THROW_GL_EXCEPTION("Failed to create texture ID for color attachment " + std::to_string(i) + " (DSA).");
        }

        // 2. 使用 DSA 方式为纹理分配存储空间和设置参数
        glTextureStorage2D(textureID, 1, colorAttachmentFormats_[i], width_, height_);
        glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // 3. 使用 DSA 方式将纹理附加到 FBO
        glNamedFramebufferTexture(id_, static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i), textureID, 0);
        
        colorAttachments_[i] = textureID;
        // 修正：显式转换加法结果
        drawBuffers[i] = static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i);
    }
    // 4. 使用 DSA 方式设置 FBO 的多重渲染目标
    glNamedFramebufferDrawBuffers(id_, static_cast<GLsizei>(colorAttachmentFormats_.size()), drawBuffers.data());
}

// --- Create Depth/Stencil Attachment (完全 DSA 化) ---
void Framebuffer::createDepthStencilAttachment()
{
    if (dsType_ == DepthStencilAttachmentType::Texture)
    {
        glCreateTextures(GL_TEXTURE_2D, 1, &depthStencilAttachment_);
        if (depthStencilAttachment_ == 0)
        {
            THROW_GL_EXCEPTION("Failed to create texture ID for depth/stencil attachment (DSA).");
        }
        glTextureStorage2D(depthStencilAttachment_, 1, GL_DEPTH24_STENCIL8, width_, height_);
        glTextureParameteri(depthStencilAttachment_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(depthStencilAttachment_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(depthStencilAttachment_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(depthStencilAttachment_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glNamedFramebufferTexture(id_, GL_DEPTH_STENCIL_ATTACHMENT, depthStencilAttachment_, 0);
    }
    else if (dsType_ == DepthStencilAttachmentType::Renderbuffer)
    {
        glCreateRenderbuffers(1, &depthStencilAttachment_);
        if (depthStencilAttachment_ == 0)
        {
            THROW_GL_EXCEPTION("Failed to create renderbuffer ID for depth/stencil attachment (DSA).");
        }
        glNamedRenderbufferStorage(depthStencilAttachment_, GL_DEPTH24_STENCIL8, width_, height_);

        glNamedFramebufferRenderbuffer(id_, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencilAttachment_);
    }
    // 如果 dsType 是 DepthStencilAttachmentType::None，则 depthStencilAttachment_ 保持为 0，不创建任何附件。
}

// --- Auxiliary function: releaseAttachments ---
void Framebuffer::releaseAttachments() {
    // 释放颜色附件纹理
    for (GLuint textureID : colorAttachments_) {
        if (glIsTexture(textureID)) {
            glDeleteTextures(1, &textureID);
        }
    }
    colorAttachments_.clear();

    // 释放深度/模板附件
    if (depthStencilAttachment_ != 0) {
        if (dsType_ == DepthStencilAttachmentType::Texture) {
            glDeleteTextures(1, &depthStencilAttachment_);
        } else if (dsType_ == DepthStencilAttachmentType::Renderbuffer) {
            glDeleteRenderbuffers(1, &depthStencilAttachment_);
        }
    }
    depthStencilAttachment_ = 0;
}

// --- deleteGlResource (实现 GLObject 的纯虚函数) ---
void Framebuffer::deleteGlResource()
{
    // 释放所有附件（纹理和渲染缓冲）
    releaseAttachments();

    // 释放 FBO 本身
    if (id_ != 0)
    {
        glDeleteFramebuffers(1, &id_);
        id_ = 0; // 删除后将 ID 归零，防止二次删除
    }
}