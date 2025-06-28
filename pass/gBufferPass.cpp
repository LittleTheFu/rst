#include "gBufferPass.h"
#include <iostream>
#include "debug_utils.h" // 确保包含调试工具
#include "sceneObject.h" // !!! 确保包含 ISceneObject 接口
#include "utilities.h"

GBufferPass::GBufferPass(int width, int height)
    : RenderPass("GBufferPass", width, height)
{
    shader_.load("shader/gPass.vert", "shader/gPass.frag");
    initGBuffer(); // 初始化 G-Buffer FBO 和纹理
}

// !!! 关键改动 !!!
// Render 方法现在接收 const std::vector<ISceneObject*>& 对象列表
void GBufferPass::Render(const std::vector<ISceneObject*>& objects, const Camera& camera)
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

    // 6. 渲染场景中的所有 ISceneObject
    // !!! 关键改动 !!!
    for (ISceneObject* obj : objects) // 遍历 ISceneObject*
    {
        if (obj == nullptr) continue;

        // 每个 ISceneObject 都需要能提供自己的模型矩阵
        shader_.setMat4("model", obj->getModelMatrix()); 
        
        // !!! 关键改动 !!!
        // 调用 ISceneObject 的 draw 方法，并传入 shader
        // ISceneObject 的 draw 方法负责绑定 VAO、VBO、EBO 并绘制几何体，
        // 同时应在内部设置材质相关的 uniform （例如 albedoColor, roughnessFactor, metallicFactor）
        // G-Buffer Shader (gPass.vert/frag) 将接收这些材质属性并写入 G-Buffer 纹理
        obj->render(shader_); 
    }

    // 7. 解绑 G-Buffer Pass 的 Framebuffer
    deactivateFramebuffer();

    GL_CHECK_ERROR(); // 检查 OpenGL 错误
}

void GBufferPass::initGBuffer()
{
    positionTexture_ = std::make_unique<Texture2D>(Utilities::generateUniqueTextureId(),width_, height_, GL_RGBA32F); // 位置
    positionTexture_->setParameters();

    normalTexture_ = std::make_unique<Texture2D>(Utilities::generateUniqueTextureId(),width_, height_, GL_RGBA32F);   // 法线
    normalTexture_->setParameters();

    albedoTexture_ = std::make_unique<Texture2D>(Utilities::generateUniqueTextureId(),width_, height_, GL_RGBA8);     // 反照率 + AO
    albedoTexture_->setParameters();

    roughnessTexture_ = std::make_unique<Texture2D>(Utilities::generateUniqueTextureId(),width_, height_, GL_RGBA8);  // 粗糙度
    roughnessTexture_->setParameters();

    metallicTexture_ = std::make_unique<Texture2D>(Utilities::generateUniqueTextureId(),width_, height_, GL_RGBA8);   // 金属度
    metallicTexture_->setParameters();

    aoTexture_ = std::make_unique<Texture2D>(Utilities::generateUniqueTextureId(),width_, height_, GL_RGBA8);        // AO
    aoTexture_->setParameters();

    // 创建深度纹理
    depthTexture_ = std::make_unique<Texture2D>(Utilities::generateUniqueTextureId(),width_, height_, GL_DEPTH_COMPONENT24);

    // 创建 Framebuffer 对象
    frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);

    // 创建 G-Buffer 纹理附件
    frameBuffer_->attachColorTexture(positionTexture_->id(), GL_COLOR_ATTACHMENT0);
    frameBuffer_->attachColorTexture(normalTexture_->id(), GL_COLOR_ATTACHMENT1);
    frameBuffer_->attachColorTexture(albedoTexture_->id(), GL_COLOR_ATTACHMENT2);
    frameBuffer_->attachColorTexture(roughnessTexture_->id(), GL_COLOR_ATTACHMENT3);
    frameBuffer_->attachColorTexture(metallicTexture_->id(), GL_COLOR_ATTACHMENT4);
    frameBuffer_->attachColorTexture(aoTexture_->id(), GL_COLOR_ATTACHMENT5); // 独立存储AO

    // 创建深度纹理附件
    frameBuffer_->attachDepthTexture(depthTexture_->id(), 0);

    // 设置绘制缓冲区 (指定哪些颜色附件会被渲染)
    std::vector<GLenum> drawBuffers = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 // 对应 position, normal, albedo, roughness, metallic, ao
    };
    frameBuffer_->setDrawBuffers(drawBuffers);

    // 检查 Framebuffer 完整性
    frameBuffer_->checkCompleteness();

    GL_CHECK_ERROR(); // 检查 OpenGL 错误
}

// 获取深度纹理
GLuint GBufferPass::getDepthTextureId() const
{
    assert(depthTexture_ && "Depth texture is not initialized!");
    return depthTexture_->id();
}

GLint GBufferPass::getPositionTextureId() const
{
    assert(positionTexture_ && "Position texture is not initialized!");
    return positionTexture_->id(); 
}

GLint GBufferPass::getNormalTextureId() const
{
    assert(normalTexture_ && "Normal texture is not initialized!");
    return normalTexture_->id();
}

GLint GBufferPass::getAlbedoTextureId() const
{
    assert(albedoTexture_ && "Albedo texture is not initialized!");
    return albedoTexture_->id();
}

GLint GBufferPass::getRoughnessTextureId() const
{
    assert(roughnessTexture_ && "Roughness texture is not initialized!");
    return roughnessTexture_->id(); 
}

GLint GBufferPass::getMetallicTextureId() const
{
    assert(metallicTexture_ && "Metallic texture is not initialized!");
    return metallicTexture_->id();
}

GLint GBufferPass::getAOTextureId() const
{
    assert(aoTexture_ && "AO texture is not initialized!");
    return aoTexture_->id();   
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

    // 重新创建 FBO 和附件
    // unique_ptr 会在赋值新对象时自动释放旧资源
    initGBuffer();

    GL_CHECK_ERROR(); // 检查 OpenGL 错误
}