#ifndef _DEPTH_OF_FIELD_PASS_H_
#define _DEPTH_OF_FIELD_PASS_H_

#include "RenderPass.h"
#include "Shader.h"
#include "screenQuad.h"
#include "texture2D.h"

class DepthOfFieldPass : public RenderPass {
public:
    DepthOfFieldPass(int width, int height);
    ~DepthOfFieldPass() override = default;

    void Render(GLuint sharpTexture,
                GLuint blurredTexture,
                GLuint depthTexture,
                float focusDepth,
                float focusRange,
                float nearPlane,
                float farPlane);

    void Resize(int width, int height) override;
    GLuint getColorTextureId() const;

private:
    void init();

private:
    std::shared_ptr<Shader> shader_;
    std::unique_ptr<Texture2D> colorTexture_;
    ScreenQuad screenQuad_;
};

#endif
