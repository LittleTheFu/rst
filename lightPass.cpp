#include "lightPass.h"
#include <iostream>
#include "uniformBuffer.h"
#include "pointLightDataForUBO.h"

LightPass::LightPass(const std::string &name) : RenderPass(name)
{
  shader_.load("shader/light.vert", "shader/light.frag"); // 假设你的屏幕 Shader 文件名为 screen.vert 和 screen.frag
}

void LightPass::Initialize(int width, int height)
{
    createFramebuffer();
    bindFramebuffer();

    // 主颜色附件
    outputTexture_ = createColorAttachment(width, height, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT0);

    // 创建 debug 深度纹理，使用浮点格式来存储深度信息，方便渲染为颜色输出
    // debugCurrentDepthTexture_
    glGenTextures(1, &debugCurrentDepthTexture_);
    glBindTexture(GL_TEXTURE_2D, debugCurrentDepthTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, debugCurrentDepthTexture_, 0);

    // debugClosestDepthTexture_
    glGenTextures(1, &debugClosestDepthTexture_);
    glBindTexture(GL_TEXTURE_2D, debugClosestDepthTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, debugClosestDepthTexture_, 0);

    // 通知 OpenGL 我们使用哪些颜色附件渲染
    std::vector<GLenum> attachments = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(static_cast<GLsizei>(attachments.size()), attachments.data());

    auto err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (err != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "ERROR::FRAMEBUFFER::Framebuffer is not complete!" << std::endl;
    }

    lightBindingPoint_ = 0;

    objectLightUBO_.create(sizeof(PointLightDataForUBO), GL_DYNAMIC_DRAW);
    objectLightUBO_.bind(lightBindingPoint_);

    // 获取 Shader 中 Uniform Block 的索引
    GLuint lightBlockIndex = glGetUniformBlockIndex(shader_.ID, "PointLightBlock");
    if (lightBlockIndex == GL_INVALID_INDEX)
    {
        std::cerr << "Error: Uniform block 'PointLightBlock' not found in screen shader!" << std::endl;
        return;
    }

    // 将 Uniform Block 绑定到相同的绑定点
    glUniformBlockBinding(shader_.ID, lightBlockIndex, lightBindingPoint_);

    objectLightUBO_.unbind(); // 解绑 UBO 是一个好习惯
    unbindFramebuffer();

    initScreenQuad();
}


// void LightPass::Initialize(int width, int height)
// {
//   createFramebuffer();
//   bindFramebuffer();

//   outputTexture_ = createColorAttachment(width, height, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT0);
//   std::vector<GLenum> attachments = {GL_COLOR_ATTACHMENT0};
//   glDrawBuffers(attachments.size(), attachments.data()); // 告诉 OpenGL 我们要渲染到哪些颜色附件
//   auto err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
//   if (err != GL_FRAMEBUFFER_COMPLETE)
//   {
//     std::cerr << "ERROR::FRAMEBUFFER::Framebuffer is not complete!" << std::endl;
//   }

//   lightBindingPoint_ = 0;

//   objectLightUBO_.create(sizeof(PointLightDataForUBO), GL_DYNAMIC_DRAW);
//   objectLightUBO_.bind(lightBindingPoint_);

//    // 获取 Shader 中 Uniform Block 的索引
//     GLuint lightBlockIndex = glGetUniformBlockIndex(shader_.ID, "PointLightBlock");
//     if (lightBlockIndex == GL_INVALID_INDEX)
//     {
//         std::cerr << "Error: Uniform block 'PointLightBlock' not found in screen shader!" << std::endl;
//         return;
//     }

//     // 将 Uniform Block 绑定到相同的绑定点
//     glUniformBlockBinding(shader_.ID, lightBlockIndex, lightBindingPoint_);

//     objectLightUBO_.unbind(); // 解绑 UBO 是一个好习惯
//     unbindFramebuffer();

//   initScreenQuad();
// }

void LightPass::Render(SceneData &sceneData, Camera &camera)
{
  std::cerr << "Warning: lightPass::Render(SceneData&, Camera&) called - consider using Render(GLuint)." << std::endl;
}

void LightPass::Render(const GLuint &textureID)
{
  // 绑定默认 Framebuffer
  // unbindFramebuffer(); // unbindFramebuffer() 继承自 RenderPass，会绑定回默认的 Framebuffer (ID 0)
  bindFramebuffer();

  glClearColor(0.0f, 0.3f, 0.0f, 1.0f);
  // 清除默认 Framebuffer
  clearBuffers(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // 使用屏幕 Shader
  shader_.use();

  // 绑定要显示的纹理
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureID);
  shader_.setInt("out_Texture", 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, debugCurrentDepthTexture_);
  shader_.setInt("out_DebugCurrentDepth", 0);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, debugClosestDepthTexture_);
  shader_.setInt("out_DebugClosestDepth", 0);

  // 渲染屏幕四边形
  renderQuad();

  // 解绑纹理
  unbindFramebuffer();
}

void LightPass::Render(const GLuint &positionTextureID,
                       const GLuint &normalTextureID,
                       const GLuint &albedoTextureID,
                       const GLuint &roughnessTextureID,
                       const GLuint &metallicTextureID,
                       const GLuint &aoTextureID,
                       const std::shared_ptr<PointLight> &light,
                       const Camera &camera,
                       const GLuint &shadowMapTextureID,
                       const Camera &shadowCamera)
{
  // 绑定默认 Framebuffer
  // unbindFramebuffer(); // unbindFramebuffer() 继承自 RenderPass，会绑定回默认的 Framebuffer (ID 0)
  bindFramebuffer();

  glClearColor(0.0f, 0.3f, 0.0f, 1.0f);
  // 清除默认 Framebuffer
  clearBuffers(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // 使用屏幕 Shader
  shader_.use();

  if (light != nullptr)
  {
    PointLightDataForUBO lightData;
    lightData.position = light->position;
    lightData.color = light->color;
    lightData.intensity = light->intensity;
    // lightData.constant = light->constant;
    // lightData.linear = light->linear;
    // lightData.quadratic = light->quadratic;

    objectLightUBO_.bind(lightBindingPoint_);
    objectLightUBO_.updateData(0, sizeof(PointLightDataForUBO), &lightData);
    objectLightUBO_.unbind();
  }

  shader_.setVec3("cameraPos", camera.Position);
  //这里应该使用shadowmap_camera的farClip,因为都一样，这里暂时省事这样写
  shader_.setFloat("shadowCameraFarClip", shadowCamera.farClip);
  

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

  glActiveTexture(GL_TEXTURE6);
  glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMapTextureID); // 假设 shadowMapTexture 是阴影贴图的纹理 ID
  shader_.setInt("shadowMapTexture", 6);                 // 假设阴影贴图绑定到纹理单元 6

  // 渲染屏幕四边形
  renderQuad();

  unbindFramebuffer();
}

void LightPass::Resize(int width, int height)
{
  setViewport(width, height);
}

void LightPass::initScreenQuad()
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

void LightPass::renderQuad()
{
  glBindVertexArray(quadVAO_);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
}