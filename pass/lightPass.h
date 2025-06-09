#ifndef LIGHT_PASS_H
#define LIGHT_PASS_H

#include "RenderPass.h"
#include "Shader.h"
#include "pointLight.h"
#include "uniformBuffer.h"
#include "camera.h"
#include "Texture2D.h"
#include "TextureCubeMap.h"
#include "screenQuad.h"

class LightPass : public RenderPass
{
public:
    LightPass(int width, int height);

    ~LightPass() override;

    void Render(GLuint positionTextureID,
                GLuint normalTextureID,
                GLuint albedoTextureID,
                GLuint roughnessTextureID,
                GLuint metallicTextureID,
                GLuint aoTextureID,
                const PointLight &light,
                const Camera &camera,
                GLuint shadowMapID);

    void Resize(int width, int height) override;

    GLuint getOutputTextureId() const { return outputTexture_->id(); }

    GLuint getDebugCurrentDepthTextureId() const { return debugCurrentDepthTexture_->id(); }
    GLuint getDebugClosestDepthTextureId() const { return debugClosestDepthTexture_->id(); }

private:
    void init();

private:
    Shader shader_;
    ScreenQuad screenQuad_;

    UniformBuffer objectLightUBO_;
    GLuint lightBindingPoint_;

    std::unique_ptr<Texture2D> outputTexture_;

    std::unique_ptr<Texture2D> debugCurrentDepthTexture_;
    std::unique_ptr<Texture2D> debugClosestDepthTexture_;
};

#endif