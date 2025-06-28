#include "combinedPass.h"
#include <iostream>
#include "debug_utils.h"
#include <cassert>
#include "utilities.h"

CombinedPass::CombinedPass(int width, int height)
    : RenderPass("CombinedPass", width, height),
      screenQuad_()
{
    shader_.load("shader/combine.vert", "shader/combine.frag");
    init();
}

void CombinedPass::Render(GLuint directLightTextureID,
                          GLuint iblTextureID,
                          GLuint gpassDepthTextureID,
                          GLuint oitAccumTextureID,
                          GLuint oitRevealTextureID,
                          GLuint skyboxTextureID,
                          GLuint ssrTextureID,
                          GLuint godRayTextureID,
                          float ssrWeight,
                          float iblWeight,
                          float lightWeight,
                          float oitWeight,
                          float godRayWeight)
{
    activateFramebuffer();

    setViewport(width_, height_);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    disableState(GL_DEPTH_TEST);

    shader_.use();

    glBindTextureUnit(0, directLightTextureID);
    shader_.setInt("directLightTexture", 0);

    glBindTextureUnit(1, iblTextureID);
    shader_.setInt("iblTexture", 1);

    glBindTextureUnit(2, gpassDepthTextureID);
    shader_.setInt("gpassDepthTexture", 2);

    glBindTextureUnit(3, oitAccumTextureID);
    shader_.setInt("accumTexture", 3);

    glBindTextureUnit(4, oitRevealTextureID);
    shader_.setInt("revealTexture", 4);

    glBindTextureUnit(5, skyboxTextureID);
    shader_.setInt("skyboxTexture", 5);

    glBindTextureUnit(6, ssrTextureID);
    shader_.setInt("ssrTexture", 6);

    glBindTextureUnit(7, godRayTextureID);
    shader_.setInt("godRayTexture", 7);

    float max = 3.0f;
    shader_.setFloat("ssrWeight", std::clamp(ssrWeight, 0.0f, max));
    shader_.setFloat("iblWeight", std::clamp(iblWeight, 0.0f, max));
    shader_.setFloat("lightWeight", std::clamp(lightWeight, 0.0f, max));
    shader_.setFloat("oitWeight", std::clamp(oitWeight, 0.0f, max));
    shader_.setFloat("godRayWeight", std::clamp(godRayWeight, 0.0f, max));

    screenQuad_.render();

    deactivateFramebuffer();

    GL_CHECK_ERROR();
}

void CombinedPass::Resize(int width, int height)
{
    RenderPass::Resize(width, height);
    setViewport(width_, height_);

    init();
}

GLuint CombinedPass::getColorTextureId() const
{
    assert(colorTexture_);

    return colorTexture_->id();
}

void CombinedPass::init()
{
    colorTexture_ = std::make_unique<Texture2D>(Utilities::generateUniqueTextureId(),width_, height_, GL_RGBA8); // 颜色
    colorTexture_->setParameters();

    depthTexture_ = std::make_unique<Texture2D>(Utilities::generateUniqueTextureId(),width_, height_, GL_DEPTH_COMPONENT24);
    frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);
    frameBuffer_->attachColorTexture(colorTexture_->id(), GL_COLOR_ATTACHMENT0);

    frameBuffer_->attachDepthTexture(depthTexture_->id(), 0);

    std::vector<GLenum> drawBuffers = {GL_COLOR_ATTACHMENT0};
    frameBuffer_->setDrawBuffers(drawBuffers);

    frameBuffer_->checkCompleteness();

    GL_CHECK_ERROR();
}