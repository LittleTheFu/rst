#include "screenPass.h"
#include <iostream>
#include "uniformBuffer.h"
#include "pointLightDataForUBO.h"

ScreenPass::ScreenPass(const std::string &name) : RenderPass(name)
{
  shader_.load("shader/screen.vert", "shader/screen.frag"); // 假设你的屏幕 Shader 文件名为 screen.vert 和 screen.frag
}

void ScreenPass::Initialize(int width, int height)
{
    // createFramebuffer();
    // bindFramebuffer();
    // unbindFramebuffer();

    initScreenQuad();
}

void ScreenPass::Render(SceneData &sceneData, Camera &camera)
{
  std::cerr << "Warning: ScreenPass::Render(SceneData&, Camera&) called - consider using Render(GLuint)." << std::endl;
}

void ScreenPass::Render(const GLuint &lightTextureID, const GLuint &iblTextureID, const GLuint &lightDepthTextureID)
{
  // 绑定默认 Framebuffer
//   bindFramebuffer();
    unbindFramebuffer();

  glClearColor(0.0f, 0.3f, 0.0f, 1.0f);
  // 清除默认 Framebuffer
  clearBuffers(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // 使用屏幕 Shader
  shader_.use();

  // 绑定要显示的纹理
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, lightTextureID);
  shader_.setInt("lightTexture", 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, iblTextureID);
  shader_.setInt("iblTexture", 1);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, lightDepthTextureID);
  shader_.setInt("lightDepthTexture", 2);

  // 渲染屏幕四边形
  renderQuad();

  // 解绑默认 Framebuffer
//   unbindFramebuffer();
}

void ScreenPass::Resize(int width, int height)
{
  setViewport(width, height);
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
}

void ScreenPass::renderQuad()
{
  glBindVertexArray(quadVAO_);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
}