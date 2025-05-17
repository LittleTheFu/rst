#include "shadowPass.h"

ShadowPass::ShadowPass() : RenderPass("ShadowPass")
{
    shader_.load("depth.vert", "depth.frag");
}

void ShadowPass::Initialize(int width, int height)
{
    createFramebuffer();
    bindFramebuffer();

    // 创建立方体纹理
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &depthAttachment_);

    // 为整个立方体贴图分配存储（只调用一次！）
    glTextureStorage2D(depthAttachment_, 1, GL_DEPTH_COMPONENT32F, width, height);

    // 设置纹理参数
    glTextureParameteri(depthAttachment_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(depthAttachment_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(depthAttachment_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(depthAttachment_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(depthAttachment_, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // 绑定纹理到帧缓冲深度附件（深度附件没有 mipmap 层级，所以level固定为0）
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthAttachment_, 0);

    GLenum err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (err != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "ERROR::FRAMEBUFFER::Framebuffer is not complete!" << std::endl;
    }

    unbindFramebuffer();
}



void ShadowPass::Render(SceneData &sceneData, Camera &camera)
{
    Eigen::Vector3f pos = camera.Position;
    Eigen::Vector3f targets[6] = {
        pos + Eigen::Vector3f(1, 0, 0),  // +X
        pos + Eigen::Vector3f(-1, 0, 0), // -X
        pos + Eigen::Vector3f(0, 1, 0),  // +Y
        pos + Eigen::Vector3f(0, -1, 0), // -Y
        pos + Eigen::Vector3f(0, 0, 1),  // +Z
        pos + Eigen::Vector3f(0, 0, -1)  // -Z
    };

    Eigen::Vector3f ups[6] = {
        Eigen::Vector3f(0, -1, 0), // +X
        Eigen::Vector3f(0, -1, 0), // -X
        Eigen::Vector3f(0, 0, 1),  // +Y
        Eigen::Vector3f(0, 0, -1), // -Y
        Eigen::Vector3f(0, -1, 0), // +Z
        Eigen::Vector3f(0, -1, 0)  // -Z
    };

    bindFramebuffer();

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    for (int face = 0; face < 6; ++face)
    {
        Eigen::Matrix4f viewMat = camera.LookAtCube(pos, targets[face], ups[face]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                               depthAttachment_, 0);

        glClear(GL_DEPTH_BUFFER_BIT);

        // 这里用shadow map分辨率，而不是sceneData.screenWidth/Height
        setViewport(sceneData.shadowMapWidth, sceneData.shadowMapHeight);

        shader_.use();
        shader_.setMat4("view", viewMat);
        shader_.setMat4("projection", camera.GetProjectionMatrix());

        for (const auto &object : sceneData.objects)
        {
            shader_.setMat4("model", object->getModelMatrix());
            object->render(shader_);
        }
    }

    unbindFramebuffer();
}
