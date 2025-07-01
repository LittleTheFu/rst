#include "lightPass.h"
#include <iostream>
#include <vector>
#include "pointLightDataForUBO.h"
#include "debug_utils.h"
#include "utilities.h"
#include "shaderManager.h"

LightPass::LightPass(int width, int height)
    : RenderPass("LightPass", width, height), screenQuad_()
{
   shader_ = ShaderManager::getInstance().loadShader("shader/light.vert", "shader/light.frag");

    init();
}

LightPass::~LightPass()
{
}

void LightPass::init()
{
    outputTexture_ = std::make_unique<Texture2D>(Utilities::generateUniqueTextureId(),width_, height_, GL_RGBA32F);
    frameBuffer_->attachColorTexture(outputTexture_->id(), GL_COLOR_ATTACHMENT0);

    debugCurrentDepthTexture_ = std::make_unique<Texture2D>(Utilities::generateUniqueTextureId(),width_, height_, GL_R32F);
    frameBuffer_->attachColorTexture(debugCurrentDepthTexture_->id(), GL_COLOR_ATTACHMENT1);

    debugClosestDepthTexture_ = std::make_unique<Texture2D>(Utilities::generateUniqueTextureId(),width_, height_, GL_R32F);
    frameBuffer_->attachColorTexture(debugClosestDepthTexture_->id(), GL_COLOR_ATTACHMENT2);

    std::vector<GLenum> drawBuffers = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    frameBuffer_->setDrawBuffers(drawBuffers);

    frameBuffer_->checkCompleteness();

    lightBindingPoint_ = shader_->getUniformBlockIndex("PointLightBlock");
    objectLightUBO_.create(sizeof(PointLightDataForUBO), GL_DYNAMIC_DRAW);
    objectLightUBO_.bindToBindingPoint(lightBindingPoint_);
}

void LightPass::Render(GLuint positionTextureID,
                       GLuint normalTextureID,
                       GLuint albedoTextureID,
                       GLuint roughnessTextureID,
                       GLuint metallicTextureID,
                       GLuint aoTextureID,
                       const PointLight &light,
                       const Camera &camera,
                       GLuint shadowMapID)
{
    activateFramebuffer();
    setViewport(width_, height_);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    clearBuffers(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    disableState(GL_DEPTH_TEST);
    shader_->use();

    glBindTextureUnit(0, positionTextureID);
    shader_->setInt("positionTexture", 0);

    glBindTextureUnit(1, normalTextureID);
    shader_->setInt("normalTexture", 1);

    glBindTextureUnit(2, albedoTextureID);
    shader_->setInt("albedoTexture", 2);

    glBindTextureUnit(3, roughnessTextureID);
    shader_->setInt("roughnessTexture", 3);

    glBindTextureUnit(4, metallicTextureID);
    shader_->setInt("metallicTexture", 4);

    glBindTextureUnit(5, aoTextureID);
    shader_->setInt("aoTexture", 5);

    glBindTextureUnit(6, shadowMapID);
    shader_->setInt("shadowMapTexture", 6);

    shader_->setVec3("cameraPos", camera.Position);
    shader_->setFloat("shadowCameraFarClip", camera.farClip);

    PointLightDataForUBO lightData;
    lightData.position = light.position;
    lightData.color = light.color;
    lightData.intensity = light.intensity;

    objectLightUBO_.updateData(0, sizeof(PointLightDataForUBO), &lightData);
    screenQuad_.render();
    deactivateFramebuffer();
    GL_CHECK_ERROR();
}

void LightPass::Resize(int width, int height)
{
    RenderPass::Resize(width, height);
    setViewport(width_, height_);
    deactivateFramebuffer();
    
    init();
}