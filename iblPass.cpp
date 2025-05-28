#include "IBLPass.h"
#include <iostream>
#include <vector>
#include "debug_utils.h"

IBLPass::IBLPass(int width, int height,
                 std::shared_ptr<TextureCubeMap> irradianceMap,
                 std::shared_ptr<TextureCubeMap> prefilterMap,
                 std::shared_ptr<Texture2D> brdfLUT)
    : RenderPass("IBLPass", width, height),
      irradianceMap_(irradianceMap),
      prefilterMap_(prefilterMap),
      brdfLUT_(brdfLUT)
{
    shader_.load("shader/ibl.vert", "shader/ibl.frag"); // IBL 着色器路径

    // 创建 IBL Pass 的输出 Framebuffer
    frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);

    // 创建一个颜色附件纹理并附加到 FBO
    auto colorOutputTex = std::make_unique<Texture2D>(width_, height_, GL_RGBA16F, 1);
    outputTexture_ = colorOutputTex->id(); // 获取其 OpenGL ID
    // 假设 IBLPass 不会自己管理这个 Texture2D 对象的生命周期，只获取 ID
    // 如果需要管理，这里需要一个成员来持有 unique_ptr/shared_ptr

    frameBuffer_->attachColorTexture(outputTexture_, GL_COLOR_ATTACHMENT0, 0);

    // 设置绘制缓冲区
    std::vector<GLenum> drawBuffers = {GL_COLOR_ATTACHMENT0};
    frameBuffer_->setDrawBuffers(drawBuffers);

    // 检查 FBO 完整性
    frameBuffer_->checkCompleteness();

    initScreenQuad();
}

// 移除不再需要的 Render(SceneData&, Camera&) 方法

// 实现基类的纯虚函数 Render()，不带参数
void IBLPass::Render(GLuint gPositionID, GLuint gNormalID, GLuint gAlbedoID,
                     GLuint gRoughnessID, GLuint gMetallicID, GLuint gAOID,
                     const Camera &camera)
{
    if (!irradianceMap_ || !prefilterMap_ || !brdfLUT_)
    {
        std::cerr << "ERROR::IBLPASS::IBL textures not set!" << std::endl;
        return;
    }

    // 1. 绑定 IBL Pass 的 Framebuffer
    activateFramebuffer();
    setViewport(width_, height_);

    // 2. 清除颜色缓冲 (IBL 贡献会叠加到前一个 Pass 的结果上，通常会清除)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    clearBuffers(GL_COLOR_BUFFER_BIT);

    // 3. 禁用深度测试
    disableState(GL_DEPTH_TEST);

    // 4. 绑定 IBL Shader
    shader_.use();

    // 5. 绑定 G-Buffer 纹理
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, gPositionID);
    glBindTextureUnit(0, gPositionID);
    shader_.setInt("gPosition", 0);

    // glActiveTexture(GL_TEXTURE1);
    // glBindTexture(GL_TEXTURE_2D, gNormalID);
    glBindTextureUnit(1, gNormalID);
    shader_.setInt("gNormal", 1);

    // glActiveTexture(GL_TEXTURE2);
    // glBindTexture(GL_TEXTURE_2D, gAlbedoID);
    glBindTextureUnit(2, gAlbedoID);
    shader_.setInt("gAlbedo", 2);

    // glActiveTexture(GL_TEXTURE3);
    // glBindTexture(GL_TEXTURE_2D, gRoughnessID);
    glBindTextureUnit(3, gRoughnessID);
    shader_.setInt("gRoughness", 3);

    // glActiveTexture(GL_TEXTURE4);
    // glBindTexture(GL_TEXTURE_2D, gMetallicID);
    glBindTextureUnit(4, gMetallicID);
    shader_.setInt("gMetallic", 4);

    // glActiveTexture(GL_TEXTURE5);
    // glBindTexture(GL_TEXTURE_2D, gAOID);
    glBindTextureUnit(5, gAOID);
    shader_.setInt("gAO", 5);

    // 6. 绑定 IBL 预计算纹理
    // irradianceMap_->activate(GL_TEXTURE6);
    irradianceMap_->activate(6);
    shader_.setInt("irradianceMap", 6);

    prefilterMap_->activate(7);
    shader_.setInt("prefilterMap", 7);

    brdfLUT_->activate(8);
    shader_.setInt("brdfLUT", 8);

    // 7. 设置 Uniform 变量
    shader_.setVec3("viewPos", camera.Position);

    // 8. 渲染全屏四边形
    renderQuad(); // 假设你有一个 renderQuad() 辅助函数来绘制全屏四边形

    // 9. 解绑纹理
    // ...

    // 10. 解绑 IBL Pass 的 Framebuffer
    deactivateFramebuffer();

    GL_CHECK_ERROR();
}

void IBLPass::Resize(int width, int height)
{
    // 如果尺寸没有变化，则无需重新创建
    if (width == width_ && height == height_)
    {
        return;
    }

    // 更新基类的尺寸
    RenderPass::Resize(width, height);

    // 重新创建 Framebuffer 和附件纹理
    deactivateFramebuffer(); // 首先解绑 FBO

    // 重新创建 Framebuffer 对象 (这将生成新的 ID 并释放旧的)
    frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);

    // 重新创建并附加颜色输出纹理
    // 这里我们假设 outputTexture_ 是 IBLPass 内部创建并管理的唯一输出纹理
    auto colorOutputTex = std::make_unique<Texture2D>(width_, height_, GL_RGBA16F, 1);
    outputTexture_ = colorOutputTex->id(); // 更新 ID
    // 同样，如果需要管理此 Texture2D 对象的生命周期，这里需要一个成员来持有 unique_ptr/shared_ptr

    frameBuffer_->attachColorTexture(outputTexture_, GL_COLOR_ATTACHMENT0, 0);

    // 重新设置绘制缓冲区
    std::vector<GLenum> drawBuffers = {GL_COLOR_ATTACHMENT0};
    frameBuffer_->setDrawBuffers(drawBuffers);

    // 检查 FBO 完整性
    frameBuffer_->checkCompleteness();
}

void IBLPass::initScreenQuad()
{
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f};

    glGenVertexArrays(1, &quadVAO_);
    glGenBuffers(1, &quadVBO_);

    glBindVertexArray(quadVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void IBLPass::renderQuad()
{
    glBindVertexArray(quadVAO_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}