#ifndef LIGHT_PASS_H
#define LIGHT_PASS_H

#include "RenderPass.h"
#include "Shader.h"
#include "pointLight.h"
#include "uniformBuffer.h"
#include "camera.h"

class LightPass : public RenderPass
{
public:
    LightPass(const std::string &name = "LightPass");
    ~LightPass() override = default;

    void Initialize(int width, int height) override;
    void Render(SceneData &sceneData, Camera &camera) override;
    void Render(const GLuint &textureID); // 用于直接渲染纹理的便捷方法
    void Render(const GLuint &positionTextureID,
                const GLuint &normalTextureID,
                const GLuint &albedoTextureID,
                const GLuint &roughnessTextureID,
                const GLuint &metallicTextureID,
                const GLuint &aoTextureID,
                const std::shared_ptr<PointLight> &light,
                const Camera& camera,
                const GLuint &shadowMapTexture);
    void Resize(int width, int height) override;
    
    GLuint getOutputTexture() const { return outputTexture_; }

private:
    GLuint quadVAO_ = 0;
    GLuint quadVBO_ = 0;

    void initScreenQuad();
    void renderQuad();

    UniformBuffer objectLightUBO_;
    GLuint lightBindingPoint_;

    GLuint outputTexture_;

};

#endif