#include "shadowPass.h"

ShadowPass::ShadowPass(int width, int height)
    : RenderPass("ShadowPass", width, height)
{
    shader_.load("shader/depth.vert", "shader/depth.frag");

    std::vector<GLenum> attachments = {GL_COLOR_ATTACHMENT0};

    // 1. 创建帧缓冲
    createFramebuffer(attachments, DepthStencilAttachmentType::None);

    // 2. 创建一个立方体深度纹理
    glGenTextures(1, &colorAttachment_);
    glBindTexture(GL_TEXTURE_CUBE_MAP, colorAttachment_);

    // 3. 为立方体每个面分配深度存储
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0,                                     // mipmap level
            GL_R32F,                 // internal format
            width, height,
            0,                                     // border
            GL_RED,                    // format
            GL_FLOAT,                              // type
            nullptr                                 // no initial data
        );
    }

    // 4. 设置纹理参数
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // 5. 解绑纹理和帧缓冲 (在 Render 函数中绑定和检查)
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unbindFramebuffer();
}

// void ShadowPass::Initialize(int width, int height)
// {
//     std::vector<GLenum> attachments = {GL_COLOR_ATTACHMENT0};

//     // 1. 创建帧缓冲
//     createFramebuffer(attachments, DepthStencilAttachmentType::None);

//     // 2. 创建一个立方体深度纹理
//     glGenTextures(1, &colorAttachment_);
//     glBindTexture(GL_TEXTURE_CUBE_MAP, colorAttachment_);

//     // 3. 为立方体每个面分配深度存储
//     for (unsigned int i = 0; i < 6; ++i)
//     {
//         glTexImage2D(
//             GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
//             0,                                     // mipmap level
//             GL_R32F,                 // internal format
//             width, height,
//             0,                                     // border
//             GL_RED,                    // format
//             GL_FLOAT,                              // type
//             nullptr                                 // no initial data
//         );
//     }

//     // 4. 设置纹理参数
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

//     // 5. 解绑纹理和帧缓冲 (在 Render 函数中绑定和检查)
//     glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
//     glBindFramebuffer(GL_FRAMEBUFFER, 0);

//     unbindFramebuffer();
// }



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
    glDepthFunc(GL_LESS);  
    glDepthMask(GL_TRUE);       // 允许写入深度缓冲


    for (int face = 0; face < 6; ++face)
    {
        Eigen::Matrix4f viewMat = camera.LookAtCube(pos, targets[face], ups[face]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                               colorAttachment_, 0);

        GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, drawBuffers);

        glClear(GL_COLOR_BUFFER_BIT);

        // 这里用shadow map分辨率，而不是sceneData.screenWidth/Height
        setViewport(sceneData.shadowMapWidth, sceneData.shadowMapHeight);

        shader_.use();
        shader_.setMat4("view", viewMat.inverse());
        shader_.setMat4("projection", camera.GetProjectionMatrix());

        shader_.setVec3("lightPos", pos);
        shader_.setFloat("farClip", camera.farClip);

        for (const auto &object : sceneData.objects)
        {
            shader_.setMat4("model", object->getModelMatrix());
            object->render(shader_);
        }
    }

    unbindFramebuffer();
}
