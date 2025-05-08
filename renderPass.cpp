#include "RenderPass.h"
#include <iostream>
#include "SDL_video.h"

RenderPass::RenderPass(const std::string &name) : name_(name),
                                                  shader_("gPass.vert", "gPass.frag"),
                                                  framebuffer_(0),
                                                  depthAttachment_(0)
{
}

void RenderPass::Resize(int width, int height)
{
    // 默认实现可以为空，派生类可以根据需要重写
}

void RenderPass::createFramebuffer()
{
    if (!SDL_GL_GetCurrentContext()) {
        std::cerr << "Error: No current OpenGL context set before creating framebuffer!" << std::endl;
        return;
    }

    // 检查 gladLoadGLLoader 是否成功
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "Error: GLAD failed to initialize in createFramebuffer!" << std::endl;
        return;
    }

    // 检查 glad_glCreateFramebuffers 函数指针是否为 NULL
    if (glad_glCreateFramebuffers == nullptr) {
        std::cerr << "Error: glad_glCreateFramebuffers function pointer is NULL!" << std::endl;
        return;
    }

    glCreateFramebuffers(1, &framebuffer_);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error after glCreateFramebuffers: " << error << " (0x" << std::hex << error << std::dec << ")" << std::endl;
    } else {
        std::cout << "Framebuffer created successfully with ID: " << framebuffer_ << std::endl;
    }
}

GLuint RenderPass::createColorAttachment(int width, int height, GLenum internalFormat, GLenum format, GLenum type, GLenum attachment)
{
    GLuint texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    glTextureStorage2D(texture, 1, internalFormat, width, height);
    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture, 0);
    colorAttachments_.push_back(texture);
    return texture;
}

GLuint RenderPass::createDepthAttachment(int width, int height, bool useTexture)
{
    if (useTexture)
    {
        glCreateTextures(GL_TEXTURE_2D, 1, &depthAttachment_);
        glTextureStorage2D(depthAttachment_, 1, GL_DEPTH_COMPONENT32F, width, height);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthAttachment_, 0);
    }
    else
    {
        glCreateRenderbuffers(1, &depthAttachment_);
        glNamedRenderbufferStorage(depthAttachment_, GL_DEPTH_COMPONENT32F, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthAttachment_);
    }
    return depthAttachment_;
}

void RenderPass::bindFramebuffer(bool read, bool draw)
{
    if (draw)
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer_);
    if (read)
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer_);
}

void RenderPass::unbindFramebuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

GLuint RenderPass::getShaderProgram() const
{
    return shader_.ID;
}

void RenderPass::setUniformInt(const std::string& name, int value)
{
    shader_.setInt(name, value);
}

void RenderPass::setUniformFloat(const std::string& name, float value)
{
    shader_.setFloat(name, value);
}

void RenderPass::setUniformVec3(const std::string& name, const Eigen::Vector3f& value)
{
    shader_.setVec3(name, value);
}

void RenderPass::setUniformMat4(const std::string& name, const Eigen::Matrix4f& value)
{
    shader_.setMat4(name, value);
}