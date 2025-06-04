#include "PostPass.h"
#include <iostream>
#include "debug_utils.h"

PostPass::PostPass(int width, int height)
    : RenderPass("PostPass", width, height),
      screenQuad_()
{
    shader_.load("shader/post.vert", "shader/post.frag"); // 假设你的屏幕 Shader 文件名为 screen.vert 和 screen.frag
}

void PostPass::Render(GLuint colorTextureID)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    setViewport(width_, height_);

    // 清除屏幕
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 绑定 Screen Shader
    shader_.use();

    // 绑定输入纹理
    glBindTextureUnit(0, colorTextureID);
    shader_.setInt("colorTexture", 0);

    screenQuad_.render(); 

    GL_CHECK_ERROR();
}

void PostPass::Resize(int width, int height)
{
    // 更新基类的尺寸
    RenderPass::Resize(width, height);
    // 设置视口大小
    setViewport(width_, height_);
}