#ifndef _SCREEN_PASS_H_
#define _SCREEN_PASS_H_

#include "RenderPass.h"
#include "Shader.h"
#include "screenQuad.h"

class ScreenPass : public RenderPass
{
public:
    // 通过构造函数注入要显示的纹理 ID
    ScreenPass(int width, int height);

    ~ScreenPass() override = default;

    // 实现基类的纯虚函数，不带参数
    void Render(GLuint colorTextureID);

    // 维持 Resize 方法
    void Resize(int width, int height) override;

private:
    std::shared_ptr<Shader> shader_;

    ScreenQuad screenQuad_;
};

#endif