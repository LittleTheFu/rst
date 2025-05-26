#include "IBLPass.h"
#include <iostream>

IBLPass::IBLPass(int width, int height)
    : RenderPass("IBLPass", width, height)
{
    shader_.load("ibl.vert", "ibl.frag"); // IBL 着色器

    std::vector<GLenum> attachments = {GL_RGBA16F};
    createFramebuffer(attachments, DepthStencilAttachmentType::None); // 无深度和模板附件

    initScreenQuad();
}

// void IBLPass::Initialize(int width, int height)
// {
//     createFramebuffer();
//     bindFramebuffer();

//     outputTexture_ = createColorAttachment(width, height, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT0);

//     std::vector<GLenum> attachments = {GL_COLOR_ATTACHMENT0};
//     glDrawBuffers(static_cast<GLsizei>(attachments.size()), attachments.data());

//     auto err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
//     if (err != GL_FRAMEBUFFER_COMPLETE)
//     {
//         std::cerr << "ERROR::FRAMEBUFFER::IBLPass Framebuffer is not complete! Error: " << err << std::endl;
//     }

//     unbindFramebuffer();

//     initScreenQuad();
// }

void IBLPass::Render(SceneData &sceneData, Camera &camera)
{
    std::cerr << "Warning: IBLPass::Render(SceneData&, Camera&) called - consider using Render(GLuint, ...)." << std::endl;
}

void IBLPass::Render(const GLuint &positionTextureID,
                      const GLuint &normalTextureID,
                      const GLuint &albedoTextureID,
                      const GLuint &roughnessTextureID,
                      const GLuint &metallicTextureID,
                      const GLuint &aoTextureID,
                      const Camera& camera)
{
    bindFramebuffer();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    clearBuffers(GL_COLOR_BUFFER_BIT);

    shader_.use();

    // 绑定 G-Buffer 纹理 (保持不变)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, positionTextureID);
    shader_.setInt("gPosition", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalTextureID);
    shader_.setInt("gNormal", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, albedoTextureID);
    shader_.setInt("gAlbedo", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, roughnessTextureID);
    shader_.setInt("gRoughness", 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, metallicTextureID);
    shader_.setInt("gMetallic", 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, aoTextureID);
    shader_.setInt("gAO", 5);

    // 绑定 IBL 预计算纹理 (使用 TextureCubeMap 实例的 use() 方法)
    if (irradianceMap_) {
        irradianceMap_->use(6); // 辐照度图绑定到纹理单元 6
        shader_.setInt("irradianceMap", 6);
    } else {
        std::cerr << "Warning: Irradiance Map not set for IBLPass!" << std::endl;
    }

    if (prefilterMap_) {
        prefilterMap_->use(7); // 预过滤环境贴图绑定到纹理单元 7
        shader_.setInt("prefilterMap", 7);
    } else {
        std::cerr << "Warning: Prefilter Map not set for IBLPass!" << std::endl;
    }

    // BRDF LUT (假设仍为 GLuint，直接绑定)
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, brdfLUT_); // BRDF LUT 是 2D 纹理
    shader_.setInt("brdfLUT", 8);


    shader_.setVec3("camPos", camera.Position);
    shader_.setFloat("maxReflectionLOD", MAX_REFLECTION_LOD);

    renderQuad();

    unbindFramebuffer();
}

void IBLPass::Resize(int width, int height)
{
    // setViewport(width, height);
    // glDeleteTextures(1, &outputTexture_);
    // outputTexture_ = createColorAttachment(width, height, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT0);
    // bindFramebuffer();
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture_, 0);
    // auto err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    // if (err != GL_FRAMEBUFFER_COMPLETE)
    // {
    //     std::cerr << "ERROR::FRAMEBUFFER::IBLPass Framebuffer resize is not complete! Error: " << err << std::endl;
    // }
    // unbindFramebuffer();
}

void IBLPass::initScreenQuad()
{
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f};

    glGenVertexArrays(1, &quadVAO_);
    glGenBuffers(1, &quadVBO_);

    glBindVertexArray(quadVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void IBLPass::renderQuad()
{
    glBindVertexArray(quadVAO_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}