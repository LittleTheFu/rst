#ifndef _SCREEN_PASS_H_
#define _SCREEN_PASS_H_

#include "RenderPass.h"
#include "Shader.h"
#include "pointLight.h"
#include "uniformBuffer.h"
#include "camera.h"

class ScreenPass : public RenderPass
{
public:
    ScreenPass(const std::string &name = "ScreenPass");
    ~ScreenPass() override = default;

    void Initialize(int width, int height) override;
    void Render(SceneData &sceneData, Camera &camera) override;
    void Render(const GLuint &lightTextureID, const GLuint &iblTextureID, const GLuint &lightDepthTextureID);
   
    void Resize(int width, int height) override;

private:
    GLuint quadVAO_ = 0;
    GLuint quadVBO_ = 0;

    void initScreenQuad();
    void renderQuad();

    UniformBuffer objectLightUBO_;
    GLuint lightBindingPoint_;

};

#endif