#include "LightPass.h"
#include <iostream>
#include <vector>

LightPass::LightPass(const std::string& name) : RenderPass(name)
{
    shader_.load("light_pass.vert", "light_pass.frag");
}

void LightPass::Initialize(int width, int height)
{
    // 创建用于渲染屏幕四边形的 VAO 和 VBO
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
    glCreateBuffers(1, &quadVBO_);
    glBindVertexArray(quadVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
}

void LightPass::Render(SceneData& sceneData, Camera& camera)
{
    bindFramebuffer(false, true); // 绑定默认帧缓冲进行写入
    clearBuffers(GL_COLOR_BUFFER_BIT);
    disableState(GL_DEPTH_TEST); // 关闭深度测试，因为我们渲染的是屏幕空间四边形

    shader_.use();

    // 绑定 G-buffer 纹理到对应的纹理单元
    std::vector<GLuint> gBufferTextures = sceneData.gBuffer->getColorAttachments();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gBufferTextures[0]); // Position
    shader_.setInt("gPosition", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gBufferTextures[1]); // Normal
    shader_.setInt("gNormal", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gBufferTextures[2]); // Albedo
    shader_.setInt("gAlbedo", 2);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gBufferTextures[3]); // Roughness
    shader_.setInt("gRoughness", 3);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, gBufferTextures[4]); // Metallic
    shader_.setInt("gMetallic", 4);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, gBufferTextures[5]); // AO
    shader_.setInt("gAo", 5);

    // 设置光源信息 (这里假设场景数据中有一个点光源)
    if (!sceneData.lights.empty())
    {
        shader_.setVec3("lightPos", sceneData.lights[0].position);
        shader_.setVec3("lightColor", sceneData.lights[0].color);
    }
    shader_.setVec3("cameraPos", camera.Position);

    renderQuad(); // 渲染覆盖屏幕的四边形

    enableState(GL_DEPTH_TEST); // 重新启用深度测试
    unbindFramebuffer();
}

void LightPass::Resize(int width, int height)
{
    // Light Pass 通常不需要特殊的 resize 处理，因为它渲染到屏幕
}

void LightPass::renderQuad()
{
    glBindVertexArray(quadVAO_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}