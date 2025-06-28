// GodRayPass.cpp

#include "godRayPass.h"
#include "debug_utils.h"
#include "utilities.h"
#include "shaderManager.h"

GodRayPass::GodRayPass(int width, int height)
    : RenderPass("GodRayPass", width, height)
{
    shader_ = ShaderManager::getInstance().loadShader("shader/godray.vert", "shader/godray.frag");
    initFramebuffer();
}

void GodRayPass::initFramebuffer()
{
    colorTexture_ = std::make_unique<Texture2D>(Utilities::generateUniqueTextureId(), width_, height_, GL_RGBA16F);
    colorTexture_->setParameters();

    frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);
    frameBuffer_->attachColorTexture(colorTexture_->id(), GL_COLOR_ATTACHMENT0);
    frameBuffer_->checkCompleteness();
}

void GodRayPass::Render(GLuint brightnessTexture,
                        const Eigen::Vector3f &lightPos,
                        const Eigen::Matrix4f &viewMatrix,
                        const Eigen::Matrix4f &projMatrix,
                        float exposure,
                        float decay,
                        float density,
                        float weight,
                        int samples)
{
    activateFramebuffer();
    setViewport(width_, height_);
    clearBuffers(GL_COLOR_BUFFER_BIT);

    Eigen::Vector2f lightScreenPos = Utilities::worldToScreenSpace(lightPos, viewMatrix, projMatrix).head<2>();

    shader_->use();
    shader_->setVec2("lightScreenPos", lightScreenPos);
    shader_->setFloat("exposure", exposure);
    shader_->setFloat("decay", decay);
    shader_->setFloat("density", density);
    shader_->setFloat("weight", weight);
    shader_->setInt("numSamples", samples);

    glBindTextureUnit(0, brightnessTexture);
    shader_->setInt("brightnessTex", 0);

    screenQuad_.render();

    deactivateFramebuffer();
    GL_CHECK_ERROR();
}

GLuint GodRayPass::getColorTextureId() const
{
    return colorTexture_->id();
}

void GodRayPass::Resize(int width, int height)
{
    if (width == width_ && height == height_)
        return;

    RenderPass::Resize(width, height);
    initFramebuffer();
}
