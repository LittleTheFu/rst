#include "ssrPass.h"
#include <cassert>
#include <iostream>
#include "debug_utils.h"
#include "utilities.h"
#include "shaderManager.h"

SSRPass::SSRPass(int width, int height)
    : RenderPass("SSRPass", width, height),
      screenQuad_()
{
    shader_ = ShaderManager::getInstance().loadShader("shader/ssr.vert", "shader/ssr.frag");
    init();
}

void SSRPass::Render(GLuint normalTextureID,
                     GLuint depthTextureID,
                     GLuint colorTextureID,
                     GLuint metallicTextureID,
                     GLuint roughnessTextureID,
                     const Eigen::Matrix4f &projectionMatrix,
                     const Eigen::Matrix4f &viewMatrix)
{
    activateFramebuffer();
    setViewport(width_, height_);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    disableState(GL_DEPTH_TEST);

    shader_->use();

    // 设置矩阵
    shader_->setMat4("uProjectionMatrix", projectionMatrix);
    shader_->setMat4("uViewMatrix", viewMatrix);
    shader_->setVec2("uScreenSize", Eigen::Vector2f(width_, height_));

    // 绑定纹理
    glBindTextureUnit(0, normalTextureID);
    shader_->setInt("gNormal", 0);

    glBindTextureUnit(1, depthTextureID);
    shader_->setInt("gDepth", 1);

    glBindTextureUnit(2, colorTextureID);
    shader_->setInt("sceneColor", 2);

    glBindTextureUnit(3, metallicTextureID);
    shader_->setInt("gMetallic", 3);

    glBindTextureUnit(4, roughnessTextureID);
    shader_->setInt("gRoughness", 4);

    screenQuad_.render();
    deactivateFramebuffer();

    GL_CHECK_ERROR();
}

void SSRPass::Resize(int width, int height)
{
    RenderPass::Resize(width, height);
    init();
}

GLuint SSRPass::getReflectionTextureId() const
{
    assert(reflectionTexture_);
    return reflectionTexture_->id();
}

void SSRPass::init()
{
    reflectionTexture_ = std::make_unique<Texture2D>(Utilities::generateUniqueTextureId(),width_, height_, GL_RGBA8);
    reflectionTexture_->setParameters();

    depthTexture_ = std::make_unique<Texture2D>(Utilities::generateUniqueTextureId(),width_, height_, GL_DEPTH_COMPONENT24);

    frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);
    frameBuffer_->attachColorTexture(reflectionTexture_->id(), GL_COLOR_ATTACHMENT0);
    frameBuffer_->attachDepthTexture(depthTexture_->id(), 0);

    std::vector<GLenum> drawBuffers = {GL_COLOR_ATTACHMENT0};
    frameBuffer_->setDrawBuffers(drawBuffers);

    frameBuffer_->checkCompleteness();
    GL_CHECK_ERROR();
}
