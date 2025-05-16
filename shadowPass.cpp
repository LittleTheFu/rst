#include "shadowPass.h"

ShadowPass::ShadowPass() : RenderPass("ShadowPass")
{
    shader_.load("depth.vert", "depth.frag");
}

void ShadowPass::Initialize(int width, int height) {
    // 1. 创建 Framebuffer
    createFramebuffer();
    bindFramebuffer();

    depthAttachment_ = createDepthAttachment(width, height, true); // 深度
    // std::cout << "GBufferPass::Initialize - Depth Attachment Texture ID: " << depthAttachment_ << std::endl;
    
    auto err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (err != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR::FRAMEBUFFER::Framebuffer is not complete!" << std::endl;
    }
    unbindFramebuffer();
}

void ShadowPass::Render(SceneData& sceneData, Camera& camera) {
    bindFramebuffer();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glClear(GL_DEPTH_BUFFER_BIT);
    setViewport(sceneData.screenWidth, sceneData.screenHeight);

    shader_.use();
    Eigen::Matrix4f viewMatrix = camera.GetViewMatrix();
    Eigen::Matrix4f projectionMatrix = camera.GetProjectionMatrix();

    shader_.setMat4("view", viewMatrix);
    shader_.setMat4("projection", projectionMatrix);

    for (const auto& object : sceneData.objects) {
        Eigen::Matrix4f modelMatrix = object->getModelMatrix();
        // std::cout << "Model Matrix:\n" << modelMatrix << std::endl;
        shader_.setMat4("model", modelMatrix);
        object->render(shader_);
    }

    unbindFramebuffer();
}