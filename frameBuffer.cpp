#include "Framebuffer.h"
#include "glException.h" // 假设你的 GL 异常类在这里
#include "debug_utils.h" // 假设你的 GL 错误检查工具在这里
#include <algorithm>     // For std::find

Framebuffer::Framebuffer(int width, int height)
    : width_(width), height_(height)
{
    if (width_ <= 0 || height_ <= 0) {
        THROW_GL_EXCEPTION("Framebuffer dimensions must be positive.");
    }

    // 使用 DSA 创建帧缓冲对象
    glCreateFramebuffers(1, &id_);
    if (id_ == 0) {
        THROW_GL_EXCEPTION("Failed to create framebuffer ID.");
    }
    GL_CHECK_ERROR();
}

void Framebuffer::deleteGlResource() {
    if (id_ != 0) {
        glDeleteFramebuffers(1, &id_);
        id_ = 0;
    }
}

void Framebuffer::activate() const {
    glBindFramebuffer(GL_FRAMEBUFFER, id_);
    GL_CHECK_ERROR();
}

void Framebuffer::deactivate() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // 绑定到默认帧缓冲 (屏幕)
    GL_CHECK_ERROR();
}

void Framebuffer::attachColorTexture(GLuint textureID, GLenum attachmentPoint, int mipLevel) {
    if (textureID == 0) {
        THROW_GL_EXCEPTION("Attempted to attach a null color texture ID.");
    }
    // 使用 DSA 函数直接将纹理附加到 FBO
    glNamedFramebufferTexture(id_, attachmentPoint, textureID, mipLevel);
    GL_CHECK_ERROR();
}

void Framebuffer::attachDepthTexture(GLuint textureID, int mipLevel) {
    if (textureID == 0) {
        THROW_GL_EXCEPTION("Attempted to attach a null depth texture ID.");
    }
    glNamedFramebufferTexture(id_, GL_DEPTH_ATTACHMENT, textureID, mipLevel);
    GL_CHECK_ERROR();
}

void Framebuffer::attachDepthStencilTexture(GLuint textureID, int mipLevel) {
    if (textureID == 0) {
        THROW_GL_EXCEPTION("Attempted to attach a null depth/stencil texture ID.");
    }
    glNamedFramebufferTexture(id_, GL_DEPTH_STENCIL_ATTACHMENT, textureID, mipLevel);
    GL_CHECK_ERROR();
}

void Framebuffer::attachDepthRenderbuffer(GLuint renderbufferID) {
    if (renderbufferID == 0) {
        THROW_GL_EXCEPTION("Attempted to attach a null depth renderbuffer ID.");
    }
    glNamedFramebufferRenderbuffer(id_, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbufferID);
    GL_CHECK_ERROR();
}

void Framebuffer::attachDepthStencilRenderbuffer(GLuint renderbufferID) {
    if (renderbufferID == 0) {
        THROW_GL_EXCEPTION("Attempted to attach a null depth/stencil renderbuffer ID.");
    }
    glNamedFramebufferRenderbuffer(id_, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbufferID);
    GL_CHECK_ERROR();
}

// --- 更新的 attachDepthCubeMapFace 方法 ---
void Framebuffer::attachDepthCubeMapFace(GLuint textureID, GLenum cubeMapFaceTarget, int mipLevel) {
    if (id_ == 0) {
        THROW_GL_EXCEPTION("Error: Framebuffer ID is 0. Framebuffer not properly created.");
    }
    if (textureID == 0) {
        THROW_GL_EXCEPTION("Attempted to attach a null CubeMap face texture ID.");
    }

    // 使用 DSA 函数 glNamedFramebufferTextureLayer 将立方体贴图的特定面作为深度附件
    // glNamedFramebufferTextureLayer 专门用于将 3D 纹理的层或立方体贴图的面附加到 FBO
    // layer 参数对应 cubeMapFaceTarget - GL_TEXTURE_CUBE_MAP_POSITIVE_X，即 0-5
    glNamedFramebufferTextureLayer(id_, GL_DEPTH_ATTACHMENT, textureID, mipLevel, cubeMapFaceTarget - GL_TEXTURE_CUBE_MAP_POSITIVE_X);
    GL_CHECK_ERROR();
}

void Framebuffer::setDrawBuffers(const std::vector<GLenum>& drawBuffers) {
    // 确保 GL_NONE 在需要时是唯一的绘制缓冲区
    if (drawBuffers.size() > 1 && std::find(drawBuffers.begin(), drawBuffers.end(), GL_NONE) != drawBuffers.end()) {
        THROW_GL_EXCEPTION("GL_NONE cannot be combined with other draw buffers.");
    }

    // 使用 DSA 函数设置绘制缓冲区
    // 如果 drawBuffers 为空，且 FBO 只有深度附件，通常意味着不向任何颜色附件写入
    if (drawBuffers.empty()) {
        glNamedFramebufferDrawBuffers(id_, 0, nullptr); // 没有颜色输出
    } else {
        glNamedFramebufferDrawBuffers(id_, static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
    }
    GL_CHECK_ERROR();
}


void Framebuffer::checkCompleteness() const {
    GLenum status = glCheckNamedFramebufferStatus(id_, GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::string errorMsg;
        switch (status) {
            case GL_FRAMEBUFFER_UNDEFINED:
                errorMsg = "GL_FRAMEBUFFER_UNDEFINED: The default framebuffer does not exist."; break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: An attachment point is not complete."; break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: No image is attached to the framebuffer."; break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: A draw buffer specifies a non-existent or incomplete attachment."; break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: The read buffer specifies a non-existent or incomplete attachment."; break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                errorMsg = "GL_FRAMEBUFFER_UNSUPPORTED: The combination of internal formats of the attached images is not supported."; break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: The number of samples or sample buffers does not match for all attached images."; break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: Not all attached layers are renderable."; break;
            default:
                errorMsg = "Unknown framebuffer completeness error: " + std::to_string(status); break;
        }
        THROW_GL_EXCEPTION("Framebuffer is not complete! Status: " + errorMsg);
    }
    GL_CHECK_ERROR();
}