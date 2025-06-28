#ifndef _BLUR_VERTICAL_PASS_H_
#define _BLUR_VERTICAL_PASS_H_

#include "RenderPass.h"
#include "Shader.h"
#include "screenQuad.h"
#include "texture2D.h"

class BlurVerticalPass : public RenderPass
{
public:
    BlurVerticalPass(int width, int height);
    ~BlurVerticalPass() override = default;

    void Render(GLuint inputTexture);
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
