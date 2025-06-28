#ifndef _BLUR_HORIZONTAL_PASS_H_
#define _BLUR_HORIZONTAL_PASS_H_

#include "RenderPass.h"
#include "Shader.h"
#include "screenQuad.h"
#include "texture2D.h"

class BlurHorizontalPass : public RenderPass
{
public:
    BlurHorizontalPass(int width, int height);
    ~BlurHorizontalPass() override = default;

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
