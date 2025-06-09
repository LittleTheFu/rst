#include "screenPass.h"
#include <iostream>
#include "debug_utils.h" // 确保包含调试工具

ScreenPass::ScreenPass(int width, int height)
    : RenderPass("ScreenPass", width, height),
      screenQuad_()
{
    shader_.load("shader/screen.vert", "shader/screen.frag");
}

// 实现基类的纯虚函数 Render()，不带参数
void ScreenPass::Render(GLuint colorTextureID)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // 绑定默认帧缓冲
    setViewport(width_, height_);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader_.use();

    glBindTextureUnit(0, colorTextureID);
    shader_.setInt("colorTexture", 0);

    screenQuad_.render(); 

    GL_CHECK_ERROR();
}

void ScreenPass::Resize(int width, int height)
{
    // 更新基类的尺寸
    RenderPass::Resize(width, height);
    // 设置视口大小
    setViewport(width_, height_);
}