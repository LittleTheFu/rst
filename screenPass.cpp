#include "ScreenPass.h"
#include <iostream>

ScreenPass::ScreenPass(const std::string& name) : RenderPass(name)
{
 shader_.load("screen.vert", "screen.frag"); // 假设你的屏幕 Shader 文件名为 screen.vert 和 screen.frag
}

void ScreenPass::Initialize(int width, int height)
{
 initScreenQuad();
}

void ScreenPass::Render(SceneData& sceneData, Camera& camera)
{
 // 默认的 Render 方法可能不需要 SceneData 和 Camera，
 // 因为 ScreenPass 通常只是显示一个已经渲染好的纹理。
 // 你可以根据你的具体需求来实现这个方法，
 // 例如，如果 ScreenPass 需要根据场景数据进行一些后处理。
 std::cerr << "Warning: ScreenPass::Render(SceneData&, Camera&) called - consider using Render(GLuint)." << std::endl;
}

void ScreenPass::Render(const GLuint& textureID)
{
 // 绑定默认 Framebuffer
 unbindFramebuffer(); // unbindFramebuffer() 继承自 RenderPass，会绑定回默认的 Framebuffer (ID 0)

 glClearColor(0.0f, 0.3f, 0.0f, 1.0f);
 // 清除默认 Framebuffer
 clearBuffers(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

 // 使用屏幕 Shader
 shader_.use();

 // 绑定要显示的纹理
 glActiveTexture(GL_TEXTURE0);
 glBindTexture(GL_TEXTURE_2D, textureID);
 shader_.setInt("screenTexture", 0);

 // 渲染屏幕四边形
 renderQuad();
}

void ScreenPass::Render(const GLuint &positionTextureID, const GLuint &normalTextureID, const GLuint &albedoTextureID, const GLuint &roughnessTextureID, const GLuint &metallicTextureID, const GLuint &aoTextureID)
{
  // 绑定默认 Framebuffer
 unbindFramebuffer(); // unbindFramebuffer() 继承自 RenderPass，会绑定回默认的 Framebuffer (ID 0)

 glClearColor(0.0f, 0.3f, 0.0f, 1.0f);
 // 清除默认 Framebuffer
 clearBuffers(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

 // 使用屏幕 Shader
 shader_.use();

 // 绑定要显示的纹理
 glActiveTexture(GL_TEXTURE0);
 glBindTexture(GL_TEXTURE_2D, positionTextureID);
 shader_.setInt("positionTexture", 0);

 glActiveTexture(GL_TEXTURE1);
 glBindTexture(GL_TEXTURE_2D, normalTextureID);
 shader_.setInt("normalTexture", 1);

 glActiveTexture(GL_TEXTURE2);
 glBindTexture(GL_TEXTURE_2D, albedoTextureID);
 shader_.setInt("albedoTexture", 2);

 glActiveTexture(GL_TEXTURE3);
 glBindTexture(GL_TEXTURE_2D, roughnessTextureID);
 shader_.setInt("roughnessTexture", 3);

 glActiveTexture(GL_TEXTURE4);
 glBindTexture(GL_TEXTURE_2D, metallicTextureID);
 shader_.setInt("metallicTexture", 4);

 glActiveTexture(GL_TEXTURE5);
 glBindTexture(GL_TEXTURE_2D, aoTextureID);
 shader_.setInt("aoTexture", 5);


 // 渲染屏幕四边形
 renderQuad();  
}

void ScreenPass::Resize(int width, int height)
{
 setViewport(width, height);
}

void ScreenPass::initScreenQuad()
{
 float quadVertices[] = {
  // positions   // texCoords
  -1.0f,  1.0f,  0.0f, 1.0f,
  -1.0f, -1.0f,  0.0f, 0.0f,
   1.0f, -1.0f,  1.0f, 0.0f,

  -1.0f,  1.0f,  0.0f, 1.0f,
   1.0f, -1.0f,  1.0f, 0.0f,
   1.0f,  1.0f,  1.0f, 1.0f
 };

 glCreateVertexArrays(1, &quadVAO_);
 glBindVertexArray(quadVAO_);

 glCreateBuffers(1, &quadVBO_);
 glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
 glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

 // 设置顶点属性指针
 // 位置属性
 glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
 glEnableVertexAttribArray(0);
 // 纹理坐标属性
 glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
 glEnableVertexAttribArray(1);

 glBindVertexArray(0);
}

void ScreenPass::renderQuad()
{
 glBindVertexArray(quadVAO_);
 glDrawArrays(GL_TRIANGLES, 0, 6);
 glBindVertexArray(0);
}