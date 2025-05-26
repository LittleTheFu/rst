#include "lightPass.h"
#include <iostream>
#include <vector> // 确保包含 vector
#include "pointLightDataForUBO.h" // 确保包含 PointLightDataForUBO

LightPass::LightPass(int width, int height,
                     const Texture2D& positionTexture,
                     const Texture2D& normalTexture,
                     const Texture2D& albedoTexture,
                     const Texture2D& roughnessTexture,
                     const Texture2D& metallicTexture,
                     const Texture2D& aoTexture,
                     const std::shared_ptr<PointLight>& light,
                     const Camera& camera,
                     const TextureCubeMap& shadowMapTexture,
                     const Camera& shadowCamera)
    : RenderPass("LightPass", width, height),
      // 初始化成员引用
      positionTexture_(positionTexture),
      normalTexture_(normalTexture),
      albedoTexture_(albedoTexture),
      roughnessTexture_(roughnessTexture),
      metallicTexture_(metallicTexture),
      aoTexture_(aoTexture),
      light_(light),
      camera_(camera),
      shadowMapTexture_(shadowMapTexture),
      shadowCamera_(shadowCamera)
{
    shader_.load("shader/light.vert", "shader/light.frag");

    // 激活 LightPass 的 Framebuffer
    activateFramebuffer();

    // 创建并附加 LightPass 的输出颜色纹理
    outputTexture_ = std::make_unique<Texture2D>(width_, height_, GL_RGBA8); // RGBA8 作为颜色输出
    frameBuffer_->attachColorTexture(outputTexture_->id(), GL_COLOR_ATTACHMENT0);

    // (可选) 创建并附加调试纹理（如果你的 shader 仍然使用它们）
    // 注意：这里的 internalFormat 假设是为了方便调试输出深度图到颜色缓冲
    // GL_R32F 适合存储原始深度值以便可视化
    debugCurrentDepthTexture_ = std::make_unique<Texture2D>(width_, height_, GL_R32F);
    frameBuffer_->attachColorTexture(debugCurrentDepthTexture_->id(), GL_COLOR_ATTACHMENT1);

    debugClosestDepthTexture_ = std::make_unique<Texture2D>(width_, height_, GL_R32F);
    frameBuffer_->attachColorTexture(debugClosestDepthTexture_->id(), GL_COLOR_ATTACHMENT2);

    // 告诉 OpenGL 我们要渲染到哪些颜色附件
    std::vector<GLenum> drawBuffers = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    frameBuffer_->setDrawBuffers(drawBuffers);

    // 检查 Framebuffer 是否完整
    frameBuffer_->checkCompleteness();

    // 解激活 Framebuffer
    deactivateFramebuffer();

    // 初始化 Uniform Buffer Object
    lightBindingPoint_ = 0; // 假设这是一个通用的绑定点
    objectLightUBO_.create(sizeof(PointLightDataForUBO), GL_DYNAMIC_DRAW);
    objectLightUBO_.bindToBindingPoint(lightBindingPoint_);

    // 获取 Shader 中 Uniform Block 的索引，并绑定到相同的绑定点
    GLuint lightBlockIndex = glGetUniformBlockIndex(shader_.ID, "PointLightBlock");
    if (lightBlockIndex == GL_INVALID_INDEX)
    {
        std::cerr << "Error: Uniform block 'PointLightBlock' not found in light shader!" << std::endl;
        // 考虑抛出异常或更严重的错误处理
    }
    glUniformBlockBinding(shader_.ID, lightBlockIndex, lightBindingPoint_);

    objectLightUBO_.unbind(); // 解绑 UBO 是一个好习惯

    initScreenQuad(); // 初始化屏幕四边形
}

LightPass::~LightPass() {
    // Unique_ptr 会自动清理纹理，UBO 和 VAO/VBO 需要手动清理
    if (quadVAO_ != 0) {
        glDeleteVertexArrays(1, &quadVAO_);
    }
    if (quadVBO_ != 0) {
        glDeleteBuffers(1, &quadVBO_);
    }
    // objectLightUBO_ 的析构函数应该会处理其资源的释放
}

void LightPass::Render() {
    // 绑定 LightPass 的 Framebuffer 作为渲染目标
    activateFramebuffer();

    // 清除颜色缓冲（LightPass 不再需要清除深度，因为它是后处理 Pass）
    // LightPass 通常只绘制一个全屏四边形，不涉及深度测试，因此通常禁用深度测试
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // 清除为黑色或其他背景色
    disableState(GL_DEPTH_TEST); // 禁用深度测试
    glClear(GL_COLOR_BUFFER_BIT); // 只清除颜色缓冲

    // 设置视口
    setViewport(width_, height_);

    // 使用 LightPass 着色器
    shader_.use();

    // 更新光源 UBO
    if (light_ != nullptr)
    {
        PointLightDataForUBO lightData;
        lightData.position = light_->position;
        lightData.color = light_->color;
        lightData.intensity = light_->intensity;

        objectLightUBO_.bindToBindingPoint(lightBindingPoint_);
        objectLightUBO_.updateData(0, sizeof(PointLightDataForUBO), &lightData);
        objectLightUBO_.unbind();
    }

    shader_.setVec3("cameraPos", camera_.Position);
    shader_.setFloat("shadowCameraFarClip", shadowCamera_.farClip); // 使用成员 shadowCamera_

    // 绑定 G-Buffer 纹理（现在使用 Texture 对象的方法）
    positionTexture_.activate(GL_TEXTURE0);
    shader_.setInt("positionTexture", 0);

    normalTexture_.activate(GL_TEXTURE1);
    shader_.setInt("normalTexture", 1);

    albedoTexture_.activate(GL_TEXTURE2);
    shader_.setInt("albedoTexture", 2);

    roughnessTexture_.activate(GL_TEXTURE3);
    shader_.setInt("roughnessTexture", 3);

    metallicTexture_.activate(GL_TEXTURE4);
    shader_.setInt("metallicTexture", 4);

    aoTexture_.activate(GL_TEXTURE5);
    shader_.setInt("aoTexture", 5);

    shadowMapTexture_.activate(GL_TEXTURE6); // 激活并绑定阴影贴图
    shader_.setInt("shadowMapTexture", 6);

    // 渲染屏幕四边形
    renderQuad();

    // 恢复状态和解绑 Framebuffer
    enableState(GL_DEPTH_TEST); // 恢复深度测试状态，以防后续 Pass 需要
    deactivateFramebuffer();
}

void LightPass::Resize(int width, int height) {
    // 调用基类 Resize 更新宽度和高度
    RenderPass::Resize(width, height);

    // 解绑当前的 FBO，确保它不是活跃的
    deactivateFramebuffer();

    // 重置 Framebuffer (会删除旧的 FBO ID 并生成新的)
    frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);
    activateFramebuffer(); // 激活新的 Framebuffer

    // 重新创建并附加所有纹理到新的 Framebuffer
    outputTexture_ = std::make_unique<Texture2D>(width_, height_, GL_RGBA8);
    frameBuffer_->attachColorTexture(outputTexture_->id(), GL_COLOR_ATTACHMENT0);

    // 重新创建调试纹理
    debugCurrentDepthTexture_ = std::make_unique<Texture2D>(width_, height_, GL_R32F);
    frameBuffer_->attachColorTexture(debugCurrentDepthTexture_->id(), GL_COLOR_ATTACHMENT1);
    debugClosestDepthTexture_ = std::make_unique<Texture2D>(width_, height_, GL_R32F);
    frameBuffer_->attachColorTexture(debugClosestDepthTexture_->id(), GL_COLOR_ATTACHMENT2);


    // 重新设置 glDrawBuffers
    std::vector<GLenum> drawBuffers = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    frameBuffer_->setDrawBuffers(drawBuffers);

    // 检查 Framebuffer 完整性
    frameBuffer_->checkCompleteness();

    deactivateFramebuffer(); // 解激活
}

void LightPass::initScreenQuad()
{
    float quadVertices[] = {
        // positions // texCoords
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    };

    glCreateVertexArrays(1, &quadVAO_);
    glBindVertexArray(quadVAO_);

    glCreateBuffers(1, &quadVBO_);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    // 设置顶点属性指针
    glEnableVertexAttribArray(0); // 位置属性
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1); // 纹理坐标属性
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

    glBindVertexArray(0);
}

void LightPass::renderQuad()
{
    glBindVertexArray(quadVAO_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}