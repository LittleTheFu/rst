#ifndef _SCREEN_PASS_H_
#define _SCREEN_PASS_H_

#include "RenderPass.h"
#include "Shader.h"
// #include "pointLight.h" // 移除，因为 ScreenPass 不直接处理光源数据
// #include "uniformBuffer.h" // 移除，假设 ScreenPass 不使用 UBO
// #include "camera.h" // 移除，因为 ScreenPass 不直接使用 Camera 数据

class ScreenPass : public RenderPass
{
public:
    // 通过构造函数注入要显示的纹理 ID
    ScreenPass(int width, int height,
               GLuint lightTextureID,
               GLuint iblTextureID,
               GLuint lightDepthTextureID);

    ~ScreenPass() override = default;

    // 实现基类的纯虚函数，不带参数
    void Render() override;

    // 维持 Resize 方法
    void Resize(int width, int height) override;

private:
    Shader shader_;

    GLuint quadVAO_ = 0;
    GLuint quadVBO_ = 0;

    void initScreenQuad();
    void renderQuad();

    // 通过构造函数注入并作为成员持有的纹理 ID
    GLuint lightTextureID_;
    GLuint iblTextureID_;
    GLuint lightDepthTextureID_;

    // 移除不再使用的 UniformBuffer 成员
    // UniformBuffer objectLightUBO_;
    // GLuint lightBindingPoint_;
};

#endif // _SCREEN_PASS_H_