#ifndef _SCREEN_PASS_H_
#define _SCREEN_PASS_H_

#include "RenderPass.h"
#include "Shader.h"
#include "screenQuad.h"
#include "texture2D.h"

class CombinedPass : public RenderPass
{
public:
    // 通过构造函数注入要显示的纹理 ID
    CombinedPass(int width, int height);

    ~CombinedPass() override = default;

    // 实现基类的纯虚函数，不带参数
    void Render(GLuint directLightTextureID,
                GLuint iblTextureID,
                GLuint gpassDepthTextureID,
                GLuint oitAccumTextureID,
                GLuint oitRevealTextureID,
                GLuint skyboxTextureID);

    // 维持 Resize 方法
    void Resize(int width, int height) override;

    GLuint getColorTextureId() const;

private:
    void init();

private:
    Shader shader_;
    std::unique_ptr<Texture2D> colorTexture_; // 存储世界空间位置
    std::unique_ptr<Texture2D> depthTexture_; // 存储深度信息（作为纹理）

    ScreenQuad screenQuad_;

    GLuint lightTextureID_;
    GLuint iblTextureID_;
    GLuint gpassDepthTextureID_;
};

#endif // _SCREEN_PASS_H_