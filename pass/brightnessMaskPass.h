#ifndef BRIGHTNESS_MASK_PASS_H
#define BRIGHTNESS_MASK_PASS_H

#include "RenderPass.h"
#include "shader.h"
#include "texture2D.h"
#include "screenQuad.h"

class BrightnessMaskPass : public RenderPass {
public:
    BrightnessMaskPass(int width, int height);
    ~BrightnessMaskPass() override = default;

    void Render(GLuint inputTexture);
    void Resize(int width, int height) override;

    GLuint getOutputTextureId() const;

private:
    std::shared_ptr<Shader> shader_;
    std::unique_ptr<Texture2D> maskTexture_;

    ScreenQuad screenQuad_;
};

#endif // BRIGHTNESS_MASK_PASS_H
