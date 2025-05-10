#ifndef SCREENPASS_H
#define SCREENPASS_H

#include "RenderPass.h"
#include "Shader.h"

class ScreenPass : public RenderPass
{
public:
    ScreenPass(const std::string &name = "ScreenPass");
    ~ScreenPass() override = default;

    void Initialize(int width, int height) override;
    void Render(SceneData &sceneData, Camera &camera) override;
    void Render(const GLuint &textureID); // 用于直接渲染纹理的便捷方法
    void Render(const GLuint &positionTextureID,
                const GLuint &normalTextureID,
                const GLuint &albedoTextureID,
                const GLuint &roughnessTextureID,
                const GLuint &metallicTextureID,
                const GLuint &aoTextureID);
    void Resize(int width, int height) override;

private:
    GLuint quadVAO_ = 0;
    GLuint quadVBO_ = 0;

    void initScreenQuad();
    void renderQuad();
};

#endif // SCREENPASS_H