#include "oitPass.h"
#include <iostream>
#include "debug_utils.h" // 确保包含调试工具
#include "pointLightDataForUBO.h"

OitPass::OitPass(int width,
                 int height,
                 std::shared_ptr<TextureCubeMap> irradianceMap,
                 std::shared_ptr<TextureCubeMap> prefilterMap,
                 std::shared_ptr<Texture2D> brdfLUT)
    : RenderPass("OitPass", width, height),
      irradianceMap_(irradianceMap),
      prefilterMap_(prefilterMap),
      brdfLUT_(brdfLUT)
{
    shader_.load("shader/oit.vert", "shader/oit.frag");

    // 初始化 Uniform Buffer Object
    lightBindingPoint_ = shader_.getUniformBlockIndex("PointLightBlock");
    objectLightUBO_.create(sizeof(PointLightDataForUBO), GL_DYNAMIC_DRAW);
    objectLightUBO_.bindToBindingPoint(lightBindingPoint_);

    init(); // 初始化 G-Buffer FBO 和纹理
}

void OitPass::Render(const std::vector<std::shared_ptr<Mesh>> &meshes,
                     const PointLight &light,
                     const Camera &camera,
                     GLuint gPassDepthTextureID)
{
    activateFramebuffer();
    setViewport(width_, height_);

    frameBuffer_->attachDepthTexture(gPassDepthTextureID, 0);
    frameBuffer_->checkCompleteness();
    GL_CHECK_ERROR();

    // 检查 Framebuffer 完整性
    // frameBuffer_->checkCompleteness();

    // glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // clearBuffers(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Eigen::Vector4f clearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearBufferfv(GL_COLOR, 0, clearColor.data());  // 使用 .data() 提供 float 指针

    // 清除 COLOR_ATTACHMENT1（reveal）
    float clearReveal = 1.0f;  // 通常是1，表示完全“未遮盖”
    glClearBufferfv(GL_COLOR, 1, &clearReveal);

    // glClear(GL_DEPTH_BUFFER_BIT);

    glDisable(GL_CULL_FACE);

    enableState(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    // glDisable(GL_BLEND);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);

    glBlendFunci(0, GL_ONE, GL_ONE); // accumulation blend target
    glBlendEquationi(0, GL_FUNC_ADD); // For accum

    glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR); // revealge blend target
    glBlendEquationi(1, GL_FUNC_ADD); // For reveal

    // glBlendFuncSeparatei(0, GL_ONE, GL_ONE, GL_ONE, GL_ONE);
    // glBlendFuncSeparatei(1, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);

    shader_.use();

    irradianceMap_->activate(6);
    shader_.setInt("irradianceMap", 6);

    prefilterMap_->activate(7);
    shader_.setInt("prefilterMap", 7);

    brdfLUT_->activate(8);
    shader_.setInt("brdfLUT", 8);

    shader_.setFloat("maxReflectionLOD", 1);

    shader_.setVec3("cameraPos", camera.getPosition());
    shader_.setMat4("projection", camera.GetProjectionMatrix());
    shader_.setMat4("view", camera.GetViewMatrix());

    PointLightDataForUBO lightData;
    lightData.position = light.position;
    lightData.color = light.color;
    lightData.intensity = light.intensity;

    objectLightUBO_.updateData(0, sizeof(PointLightDataForUBO), &lightData);


    for (const auto &mesh : meshes)
    {
        if (mesh == nullptr)
            continue;

        shader_.setMat4("model", mesh->getModelMatrix());
        mesh->render(shader_); // 绘制网格
    }

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);

    deactivateFramebuffer();
    GL_CHECK_ERROR(); // 检查 OpenGL 错误
}

void OitPass::init()
{
    accumTexture_ = std::make_unique<Texture2D>(width_, height_, GL_RGBA32F); // 颜色
    accumTexture_->setParameters();

    revealTexture_ = std::make_unique<Texture2D>(width_, height_, GL_R32F); // 颜色
    revealTexture_->setParameters();

    // 创建深度纹理
    depthTexture_ = std::make_unique<Texture2D>(width_, height_, GL_DEPTH_COMPONENT24);

    // 创建 Framebuffer 对象
    frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);

    // 创建 G-Buffer 纹理附件
    frameBuffer_->attachColorTexture(accumTexture_->id(), GL_COLOR_ATTACHMENT0);
    frameBuffer_->attachColorTexture(revealTexture_->id(), GL_COLOR_ATTACHMENT1);

    // 创建深度纹理附件
    // frameBuffer_->attachDepthTexture(depthTexture_->id(), 0);

    // 设置绘制缓冲区 (指定哪些颜色附件会被渲染)
    std::vector<GLenum> drawBuffers = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    frameBuffer_->setDrawBuffers(drawBuffers);

    // 检查 Framebuffer 完整性
    // frameBuffer_->checkCompleteness();

    GL_CHECK_ERROR(); // 检查 OpenGL 错误
}

GLint OitPass::getAccumTextureId() const
{
    assert(accumTexture_ && "Position texture is not initialized!");
    return accumTexture_->id();
}

GLint OitPass::getRevealTextureId() const
{
    assert(revealTexture_ && "alpha texture is not initialized!");
    return revealTexture_->id(); 
}

// 重写 Resize 方法以重新创建 G-Buffer 纹理
void OitPass::Resize(int width, int height)
{
    // 如果尺寸没有变化，则无需重新创建
    if (width == width_ && height == height_)
    {
        return;
    }

    // 更新基类的尺寸
    RenderPass::Resize(width, height);

    // 首先解绑 FBO
    deactivateFramebuffer();

    // 重新创建 FBO 和附件
    init();

    GL_CHECK_ERROR(); // 检查 OpenGL 错误
}