#include "DepthOfFieldPass.h"
#include "debug_utils.h"
#include <assert.h>
#include "utilities.h"

DepthOfFieldPass::DepthOfFieldPass(int width, int height)
    : RenderPass("DepthOfFieldPass", width, height),
      screenQuad_() {
    shader_.load("shader/post.vert", "shader/dof.frag");
    init();
}

void DepthOfFieldPass::Render(GLuint sharpTexture,
                              GLuint blurredTexture,
                              GLuint depthTexture,
                              float focusDepth,
                              float focusRange,
                              float nearPlane,
                              float farPlane)
{
    activateFramebuffer();
    setViewport(width_, height_);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    shader_.use();
    shader_.setInt("sharpTexture", 0);
    shader_.setInt("blurTexture", 1);
    shader_.setInt("depthTexture", 2);
    shader_.setFloat("focusDepth", focusDepth);
    shader_.setFloat("focusRange", focusRange);
    shader_.setFloat("nearPlane", nearPlane);
    shader_.setFloat("farPlane", farPlane);

    glBindTextureUnit(0, sharpTexture);
    glBindTextureUnit(1, blurredTexture);
    glBindTextureUnit(2, depthTexture);

    screenQuad_.render();

    deactivateFramebuffer();
    GL_CHECK_ERROR();
}

void DepthOfFieldPass::Resize(int width, int height) {
    RenderPass::Resize(width, height);
    init();
}

void DepthOfFieldPass::init() {
    colorTexture_ = std::make_unique<Texture2D>(Utilities::generateUniqueTextureId(),width_, height_, GL_RGBA16F);
    colorTexture_->setParameters();

    frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);
    frameBuffer_->attachColorTexture(colorTexture_->id(), GL_COLOR_ATTACHMENT0, 0);

    frameBuffer_->setDrawBuffers({GL_COLOR_ATTACHMENT0});
    frameBuffer_->checkCompleteness();

    GL_CHECK_ERROR();
}

GLuint DepthOfFieldPass::getColorTextureId() const {
    assert(colorTexture_);
    return colorTexture_->id();
}
