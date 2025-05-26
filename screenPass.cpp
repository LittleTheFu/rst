#include "screenPass.h"
#include <iostream>
// #include "uniformBuffer.h" // 移除，假设不使用
// #include "pointLightDataForUBO.h" // 移除，假设不使用
#include "debug_utils.h" // 确保包含调试工具

ScreenPass::ScreenPass(int width, int height,
                       GLuint lightTextureID,
                       GLuint iblTextureID,
                       GLuint lightDepthTextureID)
    : RenderPass("ScreenPass", width, height), // 调用基类构造函数
      lightTextureID_(lightTextureID),
      iblTextureID_(iblTextureID),
      lightDepthTextureID_(lightDepthTextureID)
{
    shader_.load("shader/screen.vert", "shader/screen.frag"); // 假设你的屏幕 Shader 文件名为 screen.vert 和 screen.frag

    // ScreenPass 通常渲染到默认帧缓冲，所以不需要创建自定义 FBO
    // base class's frameBuffer_ unique_ptr remains nullptr, which is intended.

    initScreenQuad();
}

// 移除 Render(SceneData&, Camera&) 方法

// 实现基类的纯虚函数 Render()，不带参数
void ScreenPass::Render()
{
    // ScreenPass 直接渲染到默认帧缓冲
    // 不需要调用 activateFramebuffer()，因为它会绑定默认帧缓冲 (0)
    // 或者，如果你想明确地绑定默认帧缓冲，可以调用 deactivateFramebuffer()
    // deactivateFramebuffer(); // 确保渲染到默认 FBO (即绑定 0)

    // 设置视口 (在 Resize 中已经做了，但为了确保，可以在这里再次设置)
    setViewport(width_, height_);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // 渲染到屏幕时的背景色
    // 清除默认 Framebuffer 的颜色和深度缓冲
    clearBuffers(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 使用屏幕 Shader
    shader_.use();

    // 绑定要显示的纹理 (直接使用成员变量)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, lightTextureID_);
    shader_.setInt("lightTexture", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, iblTextureID_);
    shader_.setInt("iblTexture", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, lightDepthTextureID_);
    shader_.setInt("lightDepthTexture", 2);

    // 渲染屏幕四边形
    renderQuad();

    // 不需要解绑默认 Framebuffer，因为它是默认目标
    // deactivateFramebuffer(); // 再次调用也无害，但无必要
}

void ScreenPass::Resize(int width, int height)
{
    // 更新基类的尺寸
    RenderPass::Resize(width, height);
    // 设置视口大小
    setViewport(width_, height_);
}

void ScreenPass::initScreenQuad()
{
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f};

    glCreateVertexArrays(1, &quadVAO_);
    glBindVertexArray(quadVAO_);

    glCreateBuffers(1, &quadVBO_);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    // 设置顶点属性指针
    // 位置属性
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // 纹理坐标属性
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    GL_CHECK_ERROR(); // 确保创建 VAO/VBO 没有错误
}

void ScreenPass::renderQuad()
{
    glBindVertexArray(quadVAO_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    GL_CHECK_ERROR(); // 确保绘制没有错误
}