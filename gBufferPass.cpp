#include "GBufferPass.h"
#include <iostream>
#include <vector> // 确保包含 vector

// 辅助函数：创建并附加单个颜色纹理到 Framebuffer
// 这个函数是 GBufferPass 内部的实现细节，不需要暴露到头文件
// 注意：现在它只接收 internalFormat，因为 Texture2D 构造函数只接收这个
void GBufferPass::createAndAttachColorTexture(std::unique_ptr<Texture2D>& texturePtr, GLenum internalFormat, GLenum attachmentPoint) {
    // 使用 Texture2D 的正确构造函数，只传入 width, height, internalFormat
    texturePtr = std::make_unique<Texture2D>(width_, height_, internalFormat);
    // 使用 Framebuffer 新的 attachColorTexture 方法
    frameBuffer_->attachColorTexture(texturePtr->id(), attachmentPoint);
}

GBufferPass::GBufferPass(int width, int height, SceneData& sceneData, Camera& camera)
    : RenderPass("GBufferPass", width, height),
      sceneData_(sceneData), // 初始化 SceneData 引用
      camera_(camera)        // 初始化 Camera 引用
{
    shader_.load("shader/gPass.vert", "shader/gPass.frag");

    // 激活 GBufferPass 的 Framebuffer，以便我们可以在上面附加纹理
    // 注意：这里 activateFramebuffer() 只是为了确保 FBO 存在，
    // 实际的附件操作（glNamedFramebufferTexture）不需要 FBO 处于绑定状态。
    // 但为了后续的 glDrawBuffers 和 glCheckFramebufferStatus，FBO 仍需绑定。
    activateFramebuffer();

    // 创建并附加颜色纹理
    // 修正：现在只传入 internalFormat，因为 Texture2D 构造函数只接收这个
    createAndAttachColorTexture(positionTexture_, GL_RGBA32F, GL_COLOR_ATTACHMENT0); // 位置
    createAndAttachColorTexture(normalTexture_, GL_RGBA16F, GL_COLOR_ATTACHMENT1);   // 法线
    createAndAttachColorTexture(albedoTexture_, GL_RGBA8, GL_COLOR_ATTACHMENT2); // 反照率
    createAndAttachColorTexture(roughnessTexture_, GL_RGBA8, GL_COLOR_ATTACHMENT3); // 粗糙度
    createAndAttachColorTexture(metallicTexture_, GL_RGBA8, GL_COLOR_ATTACHMENT4); // 金属度
    createAndAttachColorTexture(aoTexture_, GL_RGBA8, GL_COLOR_ATTACHMENT5); // 环境光遮蔽

    // 创建并附加深度纹理
    // 修正：使用 Texture2D 的正确构造函数，只传入 internalFormat
    // 常见的深度纹理内部格式是 GL_DEPTH_COMPONENT24 或 GL_DEPTH_COMPONENT32F
    depthTexture_ = std::make_unique<Texture2D>(width_, height_, GL_DEPTH_COMPONENT24);
    // 修正：使用 Framebuffer 新的 attachDepthTexture 方法
    frameBuffer_->attachDepthTexture(depthTexture_->id());

    // 告诉 OpenGL 我们要渲染到哪些颜色附件
    std::vector<GLenum> drawBuffers = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5
    };
    // 修正：使用 Framebuffer 新的 setDrawBuffers 方法
    frameBuffer_->setDrawBuffers(drawBuffers);

    // 检查 Framebuffer 是否完整
    // 修正：使用 Framebuffer 新的 checkCompleteness 方法
    frameBuffer_->checkCompleteness();

    // 解激活 Framebuffer，恢复到默认
    deactivateFramebuffer();
}

// GBufferPass::Render 方法保持不变，因为它已经不带参数，并使用成员变量
void GBufferPass::Render() {
    // 激活此 Pass 的 Framebuffer
    activateFramebuffer();

    // 清除缓冲
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE); // 确保深度写入是开启的
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 设置视口
    setViewport(width_, height_);

    // 使用 G-Buffer 着色器
    shader_.use();

    // 获取并设置视图和投影矩阵 (现在从成员 camera_ 获取)
    Eigen::Matrix4f viewMatrix = camera_.GetViewMatrix();
    Eigen::Matrix4f projectionMatrix = camera_.GetProjectionMatrix();
    shader_.setMat4("view", viewMatrix);
    shader_.setMat4("projection", projectionMatrix);

    // 渲染场景中的所有对象 (现在从成员 sceneData_ 获取)
    for (const auto& object : sceneData_.objects) {
        Eigen::Matrix4f modelMatrix = object->getModelMatrix();
        shader_.setMat4("model", modelMatrix);
        object->render(shader_); // 假设 object->render 接受 shader_
    }

    // 解激活 Framebuffer，恢复默认
    deactivateFramebuffer();
}

void GBufferPass::Resize(int width, int height) {
    // 首先调用基类 Resize 更新宽度和高度
    RenderPass::Resize(width, height);

    // 重新创建所有 G-Buffer 纹理以匹配新尺寸
    // 注意：这里需要先解绑 FBO，然后删除旧纹理，创建新纹理，再重新附加
    // 更稳妥的做法是 Framebuffer 有一个 clearAttachments 方法，然后重新 attach

    // 解绑当前的 FBO，确保它不是活跃的
    deactivateFramebuffer();

    // 手动删除旧的纹理对象（unique_ptr 会自动处理，当新的 unique_ptr 赋值时旧的会被销毁）
    // 重新创建并附加所有纹理到 Framebuffer
    // 修正：使用 Framebuffer 新的 attachColorTexture 和 attachDepthTexture 方法
    // 并且使用 Texture2D 的正确构造函数
    createAndAttachColorTexture(positionTexture_, GL_RGBA32F, GL_COLOR_ATTACHMENT0);
    createAndAttachColorTexture(normalTexture_, GL_RGBA16F, GL_COLOR_ATTACHMENT1);
    createAndAttachColorTexture(albedoTexture_, GL_RGBA8, GL_COLOR_ATTACHMENT2);
    createAndAttachColorTexture(roughnessTexture_, GL_RGBA8, GL_COLOR_ATTACHMENT3);
    createAndAttachColorTexture(metallicTexture_, GL_RGBA8, GL_COLOR_ATTACHMENT4);
    createAndAttachColorTexture(aoTexture_, GL_RGBA8, GL_COLOR_ATTACHMENT5);

    depthTexture_ = std::make_unique<Texture2D>(width_, height_, GL_DEPTH_COMPONENT24);
    frameBuffer_->attachDepthTexture(depthTexture_->id());

    // 重新设置 glDrawBuffers
    std::vector<GLenum> drawBuffers = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5
    };
    frameBuffer_->setDrawBuffers(drawBuffers);

    // 再次检查 Framebuffer 完整性
    frameBuffer_->checkCompleteness();

    // 不需要再次 activate/deactivate，因为 Framebuffer 已经通过 attach 方法操作了
    // 并且在 Resize 结束时，我们希望它保持未激活状态
}