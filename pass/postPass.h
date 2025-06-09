#ifndef _POST_PASS_H_
#define _POST_PASS_H_

#include "RenderPass.h"
#include "Shader.h"
#include "screenQuad.h"
#include "texture2D.h"

class PostPass : public RenderPass
{
public:
    // 通过构造函数注入要显示的纹理 ID
    PostPass(int width, int height);

    ~PostPass() override = default;

    // 实现基类的纯虚函数，不带参数
    void Render(GLuint colorTextureID);

    // 维持 Resize 方法
    void Resize(int width, int height) override;

    // 获取输出纹理 ID
    GLuint getColorTextureId() const;

private:
    void init();

private:
    Shader shader_;
    std::unique_ptr<Texture2D> colorTexture_;

    ScreenQuad screenQuad_;
};

#endif