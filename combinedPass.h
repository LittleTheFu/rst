#ifndef _COMBINED_PASS_H_
#define _COMBINED_PASS_H_

#include "RenderPass.h"
#include "Shader.h"
#include "screenQuad.h"
#include "texture2D.h"

class CombinedPass : public RenderPass
{
public:
    CombinedPass(int width, int height);

    ~CombinedPass() override = default;

    void Render(GLuint directLightTextureID,
                GLuint iblTextureID,
                GLuint gpassDepthTextureID,
                GLuint oitAccumTextureID,
                GLuint oitRevealTextureID,
                GLuint skyboxTextureID,
                GLuint ssrTextureID,
                float ssrWeight,
                float iblWeight,
                float lightWeight,
                float oitWeight);

    void Resize(int width, int height) override;
    GLuint getColorTextureId() const;

private:
    void init();

private:
    Shader shader_;
    std::unique_ptr<Texture2D> colorTexture_;
    std::unique_ptr<Texture2D> depthTexture_;

    ScreenQuad screenQuad_;

    GLuint lightTextureID_;
    GLuint iblTextureID_;
    GLuint gpassDepthTextureID_;
};

#endif