#include "GBufferPass.h"
#include <iostream>

GBufferPass::GBufferPass() : RenderPass("GBufferPass") 
{
    shader_.load("gPass.vert", "gPass.frag");
}

void GBufferPass::Initialize(int width, int height) {
    // 1. 创建 Framebuffer
    createFramebuffer();
    bindFramebuffer();

    // 2. 创建 G-Buffer 纹理附件
    positionTexture_ = createColorAttachment(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT0); // 位置
    normalTexture_ = createColorAttachment(width, height, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT1);   // 法线
    albedoTexture_ = createColorAttachment(width, height, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT2); // 反照率 (RGB), 镜面反射强度 (A)
    roughnessTexture_ = createColorAttachment(width, height, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT3);
    metallicTexture_ = createColorAttachment(width, height, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT4);
    aoTexture_ = createColorAttachment(width, height, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT5);

    // roughnessMetallicAOTexture_ = createColorAttachment(width, height, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT3); // 粗糙度 (R), 金属度 (G), AO (B)

    depthAttachment_ = createDepthAttachment(width, height, true); // 深度
    std::cout << "GBufferPass::Initialize - Depth Attachment Texture ID: " << depthAttachment_ << std::endl;

    // 3. 检查 Framebuffer 是否完整
    // bindFramebuffer();
    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthAttachment_);
    std::vector<GLenum> attachments = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3,
        GL_COLOR_ATTACHMENT4,
        GL_COLOR_ATTACHMENT5,
    };
    glDrawBuffers(attachments.size(), attachments.data()); // 告诉 OpenGL 我们要渲染到哪些颜色附件
    auto err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (err != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR::FRAMEBUFFER::Framebuffer is not complete!" << std::endl;
    }
    unbindFramebuffer();

    // 4. 加载 G-Buffer Pass 的 Shader
    // shader_.load("gPass.vert", "gPass.frag"); // 假设你的 Shader 文件名为 gbuffer.vert 和 gbuffer.frag
}

void GBufferPass::Render(SceneData& sceneData, Camera& camera) {
    bindFramebuffer();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    setViewport(sceneData.screenWidth, sceneData.screenHeight);

    shader_.use();
    shader_.setMat4("view", camera.GetViewMatrix());
    shader_.setMat4("projection", camera.GetProjectionMatrix());

    std::cout << "Camera Position: " << camera.Position.transpose() << std::endl;

    for (const auto& object : sceneData.objects) {
        Eigen::Matrix4f modelMatrix = object->getModelMatrix();
        std::cout << "Model Matrix:\n" << modelMatrix << std::endl;
        shader_.setMat4("model", modelMatrix);
        object->render(shader_);
    }

    unbindFramebuffer();
}