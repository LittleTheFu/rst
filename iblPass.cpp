#include "IBLPass.h"
#include <iostream>
#include <vector>

IBLPass::IBLPass(int width, int height,
                 GLuint positionTextureID,
                 GLuint normalTextureID,
                 GLuint albedoTextureID,
                 GLuint roughnessTextureID,
                 GLuint metallicTextureID,
                 GLuint aoTextureID,
                 const Camera& camera)
    : RenderPass("IBLPass", width, height), // 调用基类构造函数
      gPositionTextureID_(positionTextureID),
      gNormalTextureID_(normalTextureID),
      gAlbedoTextureID_(albedoTextureID),
      gRoughnessTextureID_(roughnessTextureID),
      gMetallicTextureID_(metallicTextureID),
      gAOTextureID_(aoTextureID),
      camera_(camera) // 初始化 Camera 引用
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
void IBLPass::Render() {
    // 绑定 IBL Pass 的输出 Framebuffer
    activateFramebuffer(); // 使用基类的辅助方法
    setViewport(width_, height_); // 确保视口与 FBO 尺寸匹配

    glDisable(GL_DEPTH_TEST); // IBL 通常在全屏四边形上进行，不需要深度测试
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    clearBuffers(GL_COLOR_BUFFER_BIT); // 清除颜色缓冲

    shader_.use();

    // 绑定 G-Buffer 纹理 (直接使用成员变量)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPositionTextureID_);
    shader_.setInt("gPosition", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormalTextureID_);
    shader_.setInt("gNormal", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gAlbedoTextureID_);
    shader_.setInt("gAlbedo", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gRoughnessTextureID_);
    shader_.setInt("gRoughness", 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, gMetallicTextureID_);
    shader_.setInt("gMetallic", 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, gAOTextureID_);
    shader_.setInt("gAO", 5);

    // 绑定 IBL 预计算纹理 (使用 TextureCubeMap/Texture2D 实例的 activate() 方法)
    if (irradianceMap_) {
        irradianceMap_->activate(GL_TEXTURE6); // 辐照度图绑定到纹理单元 6
        shader_.setInt("irradianceMap", 6);
    } else {
        std::cerr << "Warning: Irradiance Map not set for IBLPass!" << std::endl;
    }

    if (prefilterMap_) {
        prefilterMap_->activate(GL_TEXTURE7); // 预过滤环境贴图绑定到纹理单元 7
        shader_.setInt("prefilterMap", 7);
        shader_.setFloat("maxReflectionLOD", static_cast<float>(prefilterMap_->getMipLevels() - 1)); // 根据实际 mip 级别设置
    } else {
        std::cerr << "Warning: Prefilter Map not set for IBLPass!" << std::endl;
        shader_.setFloat("maxReflectionLOD", 4.0f); // 使用默认值，或根据需求调整
    }

    if (brdfLUT_) {
        brdfLUT_->activate(GL_TEXTURE8); // BRDF LUT 绑定到纹理单元 8
        shader_.setInt("brdfLUT", 8);
    } else {
        std::cerr << "Warning: BRDF LUT not set for IBLPass!" << std::endl;
    }

    shader_.setVec3("camPos", camera_.Position); // 使用成员 Camera 的位置

    renderQuad();

    glEnable(GL_DEPTH_TEST); // 恢复深度测试
    deactivateFramebuffer(); // 使用基类的辅助方法
}

void IBLPass::Resize(int width, int height)
{
    // 如果尺寸没有变化，则无需重新创建
    if (width == width_ && height == height_) {
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
        1.0f, 1.0f, 1.0f, 1.0f
    };

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