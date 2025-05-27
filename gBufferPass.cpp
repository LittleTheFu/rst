#include "GBufferPass.h"
#include <iostream>
#include "debug_utils.h" // 确保包含调试工具

GBufferPass::GBufferPass(int width, int height)
    : RenderPass("GBufferPass", width, height)
{
    shader_.load("shader/gPass.vert", "shader/gPass.frag");
    initGBuffer(); // 初始化 G-Buffer FBO 和纹理
}

void GBufferPass::Render(const std::vector<const Mesh*>& meshes, const Camera& camera)
{
    // 1. 绑定 G-Buffer Pass 的 Framebuffer
    activateFramebuffer();
    setViewport(width_, height_);

    // 2. 清除 G-Buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    clearBuffers(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 3. 启用深度测试和背面剔除
    enableState(GL_DEPTH_TEST);
    enableState(GL_CULL_FACE);
    glCullFace(GL_BACK); // 剔除背面

    // 4. 绑定 G-Buffer Shader
    shader_.use();

    // 5. 设置 Uniform 变量
    shader_.setMat4("projection", camera.GetProjectionMatrix());
    shader_.setMat4("view", camera.GetViewMatrix());

    // 6. 渲染场景中的所有网格
    for (const auto& mesh : meshes)
    // for (auto& mesh : meshes)
    {
        if (mesh == nullptr) continue;

        shader_.setMat4("model", mesh->getModelMatrix());

        // 绑定材质纹理 (如果材质有纹理的话)
        if (mesh->getMaterial())
        {
            auto material = mesh->getMaterial();
            if (material->getAlbedoMap()) {
                material->getAlbedoMap()->activate(GL_TEXTURE0);
                shader_.setInt("material.albedoMap", 0);
            }
            if (material->getNormalMap()) {
                material->getNormalMap()->activate(GL_TEXTURE1);
                shader_.setInt("material.normalMap", 1);
            }
            if (material->getRoughnessMap()) {
                material->getRoughnessMap()->activate(GL_TEXTURE2);
                shader_.setInt("material.roughnessMap", 2);
            }
            if (material->getMetallicMap()) {
                material->getMetallicMap()->activate(GL_TEXTURE3);
                shader_.setInt("material.metallicMap", 3);
            }
            if (material->getAmbientOcclusionMap()) {
                material->getAmbientOcclusionMap()->activate(GL_TEXTURE4);
                shader_.setInt("material.aoMap", 4);
            }
        }
        mesh->render(shader_); // 绘制网格
    }

    // 7. 解绑 G-Buffer Pass 的 Framebuffer
    deactivateFramebuffer();

    GL_CHECK_ERROR(); // 检查 OpenGL 错误
}

void GBufferPass::initGBuffer()
{
    gPosition_ = std::make_unique<Texture2D>(width_, height_, GL_RGBA32F); // 位置
    // gPosition_->allocateStorage(1);
    // gPosition_->setParameters();

    gNormal_ = std::make_unique<Texture2D>(width_, height_, GL_RGBA32F);   // 法线
    // gNormal_->allocateStorage(1);
    // gNormal_->setParameters();

    gAlbedo_ = std::make_unique<Texture2D>(width_, height_, GL_RGBA8); // 反照率 + AO
    // gAlbedo_->allocateStorage(1);
    // gAlbedo_->setParameters();

    gRoughness_ = std::make_unique<Texture2D>(width_, height_, GL_R8); // 粗糙度
    // gRoughness_->allocateStorage(1);
    // gRoughness_->setParameters();

    gMetallic_ = std::make_unique<Texture2D>(width_, height_, GL_R8); // 金属度
    // gMetallic_->allocateStorage(1);
    // gMetallic_->setParameters();

    gMetallic_ = std::make_unique<Texture2D>(width_, height_, GL_R8);  // 金属度
    // gMetallic_->allocateStorage(1);
    // gMetallic_->setParameters();

    gAO_ = std::make_unique<Texture2D>(width_, height_, GL_R8);       // AO
    // gAO_->allocateStorage(1);
    // gAO_->setParameters();

    // 创建深度纹理
    depthTexture_ = std::make_unique<Texture2D>(width_, height_, GL_DEPTH_COMPONENT24);

    // 创建 Framebuffer 对象
    frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);

    // 创建 G-Buffer 纹理附件
    // 位置 (RGBA32F)
    frameBuffer_->attachColorTexture(gPosition_->id(), GL_COLOR_ATTACHMENT0);
    // 法线 (RGBA32F)
    frameBuffer_->attachColorTexture(gNormal_->id(), GL_COLOR_ATTACHMENT1);
    // 反照率 + AO (RGBA8) - AO 可存储在 Alpha 通道
    frameBuffer_->attachColorTexture(gAlbedo_->id(), GL_COLOR_ATTACHMENT2);
    // 粗糙度 (R8)
    frameBuffer_->attachColorTexture(gRoughness_->id(), GL_COLOR_ATTACHMENT3);
    // 金属度 (R8)
    frameBuffer_->attachColorTexture(gMetallic_->id(), GL_COLOR_ATTACHMENT4);
    // AO (R8) - 如果你之前在 albedo 的 alpha 通道存储了AO，这里可以不独立存储
    // 我这里独立存储AO，如果之前albedo的alpha通道用作别的或者不需要存储AO，可以移除
    // frameBuffer_->attachColorTexture(gAO_, GL_COLOR_ATTACHMENT5, 0, GL_R8, GL_RED, GL_UNSIGNED_BYTE);
    // 更新：根据gAlbedo注释，AO可能已经包含在albedo的alpha通道，或者是一个单独的AO贴图。
    // 如果是独立的AO贴图，且你想作为G-Buffer输出，可以像下面这样：
    frameBuffer_->attachColorTexture(gAO_->id(), GL_COLOR_ATTACHMENT5);


    // 创建深度纹理附件
    frameBuffer_->attachDepthTexture(depthTexture_->id(), 0);

    // 设置绘制缓冲区 (指定哪些颜色附件会被渲染)
    std::vector<GLenum> drawBuffers = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 // 对应 gPosition, gNormal, gAlbedo, gRoughness, gMetallic, gAO
    };
    frameBuffer_->setDrawBuffers(drawBuffers);

    // 检查 Framebuffer 完整性
    frameBuffer_->checkCompleteness();

    GL_CHECK_ERROR(); // 检查 OpenGL 错误
}

GLuint GBufferPass::getColorAttachment(int index) const
{
    switch (index)
    {
    case 0: return gPosition_->id();
    case 1: return gNormal_->id();
    case 2: return gAlbedo_->id();
    case 3: return gRoughness_->id();
    case 4: return gMetallic_->id();
    case 5: return gAO_->id();
    default: return 0;
    }
}

// 获取深度纹理
GLuint GBufferPass::getDepthAttachment() const
{
    return depthTexture_->id();
}

// 重写 Resize 方法以重新创建 G-Buffer 纹理
void GBufferPass::Resize(int width, int height)
{
    // 如果尺寸没有变化，则无需重新创建
    if (width == width_ && height == height_) {
        return;
    }

    // 更新基类的尺寸
    RenderPass::Resize(width, height);

    // 首先解绑 FBO
    deactivateFramebuffer();

    // 删除旧的纹理
    // glDeleteTextures(1, &gPosition_);
    // glDeleteTextures(1, &gNormal_);
    // glDeleteTextures(1, &gAlbedo_);
    // glDeleteTextures(1, &gRoughness_);
    // glDeleteTextures(1, &gMetallic_);
    // glDeleteTextures(1, &gAO_);
    // glDeleteTextures(1, &depthTexture_);

    // 重新创建 FBO 和附件
    initGBuffer();

    GL_CHECK_ERROR(); // 检查 OpenGL 错误
}