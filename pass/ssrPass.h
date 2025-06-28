#ifndef _SSR_PASS_H_
#define _SSR_PASS_H_

#include "RenderPass.h"
#include "Shader.h"
#include "screenQuad.h"
#include "texture2D.h"
#include <Eigen/Dense>


class SSRPass : public RenderPass {
public:
    SSRPass(int width, int height);
    ~SSRPass() override = default;

    void Render(GLuint normalTextureID,
                GLuint depthTextureID,
                GLuint colorTextureID,
                GLuint metallicTextureID,
                GLuint roughnessTextureID,
                const Eigen::Matrix4f &projectionMatrix,
                const Eigen::Matrix4f &viewMatrix);

    void Resize(int width, int height) override;
    GLuint getReflectionTextureId() const;

private:
    void init();

private:
    std::shared_ptr<Shader> shader_;
    std::unique_ptr<Texture2D> reflectionTexture_;
    std::unique_ptr<Texture2D> depthTexture_;
    ScreenQuad screenQuad_;
};

#endif
