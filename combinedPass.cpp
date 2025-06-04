#include "combinedPass.h"
#include <iostream>
// #include "uniformBuffer.h" // 移除，假设不使用
// #include "pointLightDataForUBO.h" // 移除，假设不使用
#include "debug_utils.h" // 确保包含调试工具

CombinedPass::CombinedPass(int width, int height)
    : RenderPass("CombinedPass", width, height),
      screenQuad_()
{
    shader_.load("shader/combine.vert", "shader/combine.frag"); // 假设你的屏幕 Shader 文件名为 screen.vert 和 screen.frag

    // CombinedPass 通常渲染到默认帧缓冲，所以不需要创建自定义 FBO
    // base class's frameBuffer_ unique_ptr remains nullptr, which is intended.

    // initScreenQuad();
}

// 移除 Render(SceneData&, Camera&) 方法

// 实现基类的纯虚函数 Render()，不带参数
void CombinedPass::Render(GLuint directLightTextureID,
                        GLuint iblTextureID,
                        GLuint gpassDepthTextureID,
                        GLuint oitAccumTextureID,
                        GLuint oitRevealTextureID,
                        GLuint skyboxTextureID)
{
    // CombinedPass 通常直接渲染到默认帧缓冲 (屏幕)
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // 绑定默认帧缓冲
    setViewport(width_, height_);

    // 清除屏幕
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glBlendEquation(GL_FUNC_ADD);

    // 禁用深度测试
    disableState(GL_DEPTH_TEST);

    // 绑定 Screen Shader
    shader_.use();

    // 绑定输入纹理
    glBindTextureUnit(0, directLightTextureID);
    shader_.setInt("directLightTexture", 0);

    glBindTextureUnit(1, iblTextureID);
    shader_.setInt("iblTexture", 1);

    glBindTextureUnit(2, gpassDepthTextureID);
    shader_.setInt("gpassDepthTexture", 2);

    glBindTextureUnit(3, oitAccumTextureID);
    shader_.setInt("accumTexture", 3);

    glBindTextureUnit(4, oitRevealTextureID);
    shader_.setInt("revealTexture", 4);

    glBindTextureUnit(5, skyboxTextureID);
    shader_.setInt("skyboxTexture", 5);

    screenQuad_.render(); 

    GL_CHECK_ERROR();
}

void CombinedPass::Resize(int width, int height)
{
    // 更新基类的尺寸
    RenderPass::Resize(width, height);
    // 设置视口大小
    setViewport(width_, height_);
}