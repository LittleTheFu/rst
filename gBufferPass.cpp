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
    albedoSpecularTexture_ = createColorAttachment(width, height, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT2); // 反照率 (RGB), 镜面反射强度 (A)
    roughnessMetallicAOTexture_ = createColorAttachment(width, height, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT3); // 粗糙度 (R), 金属度 (G), AO (B)

    depthAttachment_ = createDepthAttachment(width, height); // 深度

    // 3. 检查 Framebuffer 是否完整
    // bindFramebuffer();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthAttachment_);
    std::vector<GLenum> attachments = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
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
    // 1. 绑定 G-Buffer Framebuffer
    bindFramebuffer();
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);


    clearBuffers(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    setViewport(sceneData.screenWidth, sceneData.screenHeight);

    // 2. 绑定 G-Buffer Shader
    shader_.use();

    // 3. 设置 Uniform 变量
    shader_.setMat4("view", camera.GetViewMatrix());
    shader_.setMat4("projection", camera.GetProjectionMatrix());

    // 4. 渲染场景中的所有物体 (这里需要根据你的场景数据结构来修改)
    for (const auto& object : sceneData.objects) {
        // shader_.setMat4("model", object->getModelMatrix()); // 假设你的物体有模型矩阵
        // 设置其他材质相关的 Uniforms (例如，如果有)
        object->render(shader_); // 假设你的 object 有 mesh 成员，并且有 draw 方法
    }

    // 5. 解绑 Framebuffer
    unbindFramebuffer();
}