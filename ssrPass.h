#ifndef _SSR_PASS_H_
#define _SSR_PASS_H_

#include "RenderPass.h"
#include "Shader.h"
#include "screenQuad.h"
#include "texture2D.h"

class SSRPass : public RenderPass {
public:
    SSRPass(int width, int height);
    ~SSRPass() override = default;

    void Render(GLuint normalTextureID,
                GLuint depthTextureID,
                GLuint colorTextureID,
                const glm::mat4& projectionMatrix,
                const glm::mat4& viewMatrix);

    void Resize(int width, int height) override;
    GLuint getReflectionTextureId() const;

private:
    void init();

private:
    Shader shader_;
    std::unique_ptr<Texture2D> reflectionTexture_;
    std::unique_ptr<Texture2D> depthTexture_;
    ScreenQuad screenQuad_;
};

#endif
