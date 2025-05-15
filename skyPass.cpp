#include "skyPass.h"

SkyPass::SkyPass() : RenderPass("SkyPass")
{
    shader_.load("skyBox.vert", "skyBox.frag");
}


void SkyPass::Render(SceneData &sceneData, Camera &camera)
{
    if (sceneData.skybox == nullptr)
        return;

    bindFramebuffer();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    clearBuffers(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    setViewport(sceneData.screenWidth, sceneData.screenHeight);

     // 2. 绑定 G-Buffer Shader
    shader_.use();

    // 3. 设置 Uniform 变量
    
    shader_.setMat4("view", camera.GetViewMatrix());
    shader_.setMat4("projection", camera.GetProjectionMatrix());

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    // 4. 绑定天空盒纹理
    sceneData.skybox->render(shader_);

    glDepthMask(GL_TRUE);
    // 5. 解绑纹理
    unbindFramebuffer();
}

void SkyPass::Initialize(int width, int height)
{
    // 1. 创建 Framebuffer
    createFramebuffer();
    bindFramebuffer();

    // 2. 创建颜色附件纹理
    glCreateTextures(GL_TEXTURE_2D, 1, &colorTexture_);
    glBindTexture(GL_TEXTURE_2D, colorTexture_);

    // 3. 配置颜色附件纹理
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0); // 解绑纹理

    // 4. 将颜色附件纹理关联到 Framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture_, 0);

    // 5. 创建深度渲染缓冲对象 (仍然可以使用 RBO 作为深度附件)
    glCreateRenderbuffers(1, &depthRBO_);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRBO_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRBO_);

    // 6. 检查 Framebuffer 状态
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Framebuffer not complete! Status: " << status << std::endl;
    }

    // 7. 解绑 Framebuffer
    unbindFramebuffer();
}

