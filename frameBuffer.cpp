// Framebuffer.cpp
#include "Framebuffer.h"
#include <glad/glad.h>

Framebuffer::Framebuffer(int width, int height, int numColorAttachments, GLenum colorFormat, bool depthStencil)
    : width_(width), height_(height) {
    generateId(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, id_);

    createColorAttachments(numColorAttachments, colorFormat);
    createDepthStencilAttachment(depthStencil);

    // 检查帧缓冲是否完整
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        // 处理帧缓冲不完整的错误，例如抛出异常或记录错误日志
        fprintf(stderr, "Framebuffer is not complete!\n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer::~Framebuffer() {
    release();
}

void Framebuffer::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, id_);
    glViewport(0, 0, width_, height_); // 可选：绑定时设置视口
}

void Framebuffer::unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint Framebuffer::getColorAttachment(int index) const {
    if (index >= 0 && index < colorAttachments_.size()) {
        return colorAttachments_[index];
    }
    return 0; // 或者抛出异常
}

GLuint Framebuffer::getDepthStencilAttachment() const {
    return depthStencilAttachment_;
}

void Framebuffer::resize(int width, int height) {
    width_ = width;
    height_ = height;
    glBindFramebuffer(GL_FRAMEBUFFER, id_);
    // 需要重新创建和附加附件，因为纹理和渲染缓冲对象的大小已改变
    createColorAttachments(colorAttachments_.size(), GL_RGBA); // 假设颜色格式不变
    createDepthStencilAttachment(depthStencilAttachment_ != 0); // 如果之前有深度/模板附件，则重新创建

    // 检查帧缓冲是否完整
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "Framebuffer is not complete after resize!\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::createColorAttachments(int num, GLenum format) {
    colorAttachments_.resize(num);
    for (GLuint& textureID : colorAttachments_) {
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr); // 假设内部格式是 RGBA
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + &textureID - &colorAttachments_[0], GL_TEXTURE_2D, textureID, 0);
    }
}

void Framebuffer::createDepthStencilAttachment(bool useTexture) {
    depthStencilAttachment_ = 0;
    glBindFramebuffer(GL_FRAMEBUFFER, id_);
    if (useTexture) {
        glGenTextures(1, &depthStencilAttachment_);
        glBindTexture(GL_TEXTURE_2D, depthStencilAttachment_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width_, height_, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilAttachment_, 0);
    } else {
        GLuint rboID;
        glGenRenderbuffers(1, &rboID);
        glBindRenderbuffer(GL_RENDERBUFFER, rboID);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width_, height_);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboID);
        depthStencilAttachment_ = rboID; // 存储 RBO 的 ID
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::release() {
    glBindFramebuffer(GL_FRAMEBUFFER, id_);

    // 释放颜色附件纹理
    for (GLuint textureID : colorAttachments_) {
        glDeleteTextures(1, &textureID);
    }
    colorAttachments_.clear();

    // 释放深度/模板附件
    if (glIsTexture(depthStencilAttachment_)) {
        glDeleteTextures(1, &depthStencilAttachment_);
    } else if (glIsRenderbuffer(depthStencilAttachment_)) {
        glDeleteRenderbuffers(1, &depthStencilAttachment_);
    }
    depthStencilAttachment_ = 0;

    // 释放 FBO 本身
    if (id_ != 0) {
        glDeleteFramebuffers(1, &id_);
        id_ = 0;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}