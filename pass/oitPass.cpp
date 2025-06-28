#include "oitPass.h"
#include <iostream>
#include "debug_utils.h" // 确保包含调试工具
#include "pointLightDataForUBO.h"
#include "sceneObject.h" // !!! 确保包含 ISceneObject 接口
#include "utilities.h"

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

    init(); // 初始化 FBO 和纹理
}

// !!! 关键改动 !!!
// Render 方法现在接收 const std::vector<ISceneObject*>& 对象列表
void OitPass::Render(const std::vector<ISceneObject*> &objects, // 传入 ISceneObject* 列表
                     const PointLight &light,
                     const Camera &camera,
                     GLuint gPassDepthTextureID)
{
    activateFramebuffer();
    setViewport(width_, height_);

    // 将 G-Buffer 的深度纹理附加到 OIT Pass 的 FBO 上，作为只读深度缓冲区
    // 这样透明物体就可以利用已有的深度信息进行排序（虽然 OIT 是序独立，但有时仍需比较）
    // 或者仅仅是确保深度测试能够参照场景中已有的不透明物体的深度。
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gPassDepthTextureID, 0);
    frameBuffer_->checkCompleteness(); // 每次附件改变后都检查 FBO 完整性
    GL_CHECK_ERROR();

    // 清除颜色附件
    Eigen::Vector4f clearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearBufferfv(GL_COLOR, 0, clearColor.data());  // 清除 accumTexture_
    float clearReveal = 1.0f; // 清除 revealTexture_
    glClearBufferfv(GL_COLOR, 1, &clearReveal);

    glDisable(GL_CULL_FACE); // 透明物体通常不进行背面剔除，以免影响两面显示

    enableState(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE); // OIT 渲染时不写入深度缓冲区，只读取
    glDepthFunc(GL_LESS); // 通常使用 GL_LESS，透明物体应在不透明物体之后渲染

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD); // 累积颜色和权重
    
    // 对于 GL_COLOR_ATTACHMENT0 (Accumulation Buffer)
    glBlendFunci(0, GL_ONE, GL_ONE); // accum: src * 1 + dest * 1
    glBlendEquationi(0, GL_FUNC_ADD); 

    // 对于 GL_COLOR_ATTACHMENT1 (Revealage Buffer)
    glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR); // reveal: src * 0 + dest * (1 - src_color)
    glBlendEquationi(1, GL_FUNC_ADD); 

    shader_.use();

    // 绑定 IBL 纹理
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

    // 更新光源 UBO
    PointLightDataForUBO lightData;
    lightData.position = light.position;
    lightData.color = light.color;
    lightData.intensity = light.intensity;
    objectLightUBO_.updateData(0, sizeof(PointLightDataForUBO), &lightData);


    // !!! 关键改动 !!!
    // 渲染所有透明的 ISceneObject
    for (ISceneObject* obj : objects) // 遍历 ISceneObject*
    {
        if (obj == nullptr)
            continue;

        // 设置模型矩阵
        shader_.setMat4("model", obj->getModelMatrix());
        
        // 调用 ISceneObject 的 draw 方法，并传入 OIT 着色器
        // ISceneObject::draw 负责绑定 VAO、VBO、EBO 并绘制几何体
        // 并且应该设置透明材质所需的 uniform，例如 albedoColor, opacity 等
        obj->render(shader_); 
    }

    // 恢复 OpenGL 状态
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE); // 重新开启深度写入，以便后续 Pass 正常进行
    glEnable(GL_CULL_FACE); // 恢复背面剔除（如果这是默认状态）
    glCullFace(GL_BACK); // 恢复背面剔除（如果这是默认状态）

    deactivateFramebuffer();
    GL_CHECK_ERROR(); // 检查 OpenGL 错误
}

void OitPass::init()
{
    accumTexture_ = std::make_unique<Texture2D>(Utilities::generateUniqueTextureId(),width_, height_, GL_RGBA32F); // 累积颜色和不透明度
    accumTexture_->setParameters();

    revealTexture_ = std::make_unique<Texture2D>(Utilities::generateUniqueTextureId(),width_, height_, GL_R32F);   // 透明度（revealage）
    revealTexture_->setParameters();

    // 如果 OIT Pass 不需要自己的独立深度纹理，并且只使用 G-Buffer 的深度
    // 那么这里创建 depthTexture_ 可能是多余的。
    // 我暂时保留，但请你确认其必要性。
    depthTexture_ = std::make_unique<Texture2D>(Utilities::generateUniqueTextureId(),width_, height_, GL_DEPTH_COMPONENT24);

    // 创建 Framebuffer 对象
    frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);

    // 附件 OIT 颜色纹理
    frameBuffer_->attachColorTexture(accumTexture_->id(), GL_COLOR_ATTACHMENT0);
    frameBuffer_->attachColorTexture(revealTexture_->id(), GL_COLOR_ATTACHMENT1);

    // OIT Pass 通常不写入自己的深度缓冲区，或者说，它使用传入的 G-Buffer 深度。
    // 所以这里不应该有独立的深度纹理附件。
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture_->id(), 0); // 不要在这里附件，或者确认用途

    // 设置绘制缓冲区
    std::vector<GLenum> drawBuffers = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    frameBuffer_->setDrawBuffers(drawBuffers);

    // 检查 Framebuffer 完整性 (在实际渲染前，当深度纹理被动态附件时才进行完整性检查)
    // frameBuffer_->checkCompleteness(); 

    GL_CHECK_ERROR(); // 检查 OpenGL 错误
}

GLint OitPass::getAccumTextureId() const
{
    assert(accumTexture_ && "Accumulation texture is not initialized!");
    return accumTexture_->id();
}

GLint OitPass::getRevealTextureId() const
{
    assert(revealTexture_ && "Revealage texture is not initialized!");
    return revealTexture_->id(); 
}

// 重写 Resize 方法以重新创建 OIT 纹理
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