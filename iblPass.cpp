#include "IBLPass.h"
#include <iostream>

// 注意：Shader 文件名假定为 ibl.vert 和 ibl.frag
IBLPass::IBLPass(const std::string &name) : RenderPass(name)
{
    shader_.load("ibl.vert", "ibl.frag"); // IBL 着色器
}

void IBLPass::Initialize(int width, int height)
{
    createFramebuffer();
    bindFramebuffer();

    // IBL Pass 的输出纹理，存储计算好的间接光照结果
    // 通常使用浮点格式，因为 PBR 计算在线性空间，可能包含高亮度值
    // 之后会进行 Tone Mapping 和 Gamma Correction
    outputTexture_ = createColorAttachment(width, height, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT0);

    // 不需要深度附件，因为是屏幕空间着色，深度信息来自 G-Buffer 的 Position 纹理
    // 但如果你的 RenderPass::createFramebuffer 默认创建了深度附件，这里需要确保它不会干扰。
    // 如果需要显式禁用深度附件，可以在 bindFramebuffer() 后进行设置：
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0); // 解绑深度附件

    // 通知 OpenGL 我们使用哪些颜色附件渲染 (这里只有一个)
    std::vector<GLenum> attachments = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(static_cast<GLsizei>(attachments.size()), attachments.data());

    auto err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (err != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "ERROR::FRAMEBUFFER::IBLPass Framebuffer is not complete! Error: " << err << std::endl;
    }

    unbindFramebuffer(); // 解绑 FBO

    initScreenQuad(); // 初始化屏幕四边形
}

void IBLPass::Render(SceneData &sceneData, Camera &camera)
{
    // 这个重载函数通常不会被IBLPass直接使用，因为IBLPass需要G-Buffer纹理作为输入
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
    bindFramebuffer(); // 绑定 IBL Pass 的 FBO

    // 清除当前的 FBO
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // IBL 通常是叠加效果，所以背景清为黑色
    clearBuffers(GL_COLOR_BUFFER_BIT); // IBLPass只渲染颜色，不需要深度清除

    shader_.use(); // 使用 IBL 着色器

    // 绑定 G-Buffer 纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, positionTextureID);
    shader_.setInt("gPosition", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalTextureID);
    shader_.setInt("gNormal", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, albedoTextureID);
    shader_.setInt("gAlbedo", 2); // albedo也包含了基色

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, roughnessTextureID);
    shader_.setInt("gRoughness", 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, metallicTextureID);
    shader_.setInt("gMetallic", 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, aoTextureID);
    shader_.setInt("gAO", 5);

    // 绑定 IBL 预计算纹理
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap_); // 辐照度图是立方体贴图
    shader_.setInt("irradianceMap", 6);

    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap_); // 预过滤环境贴图是立方体贴图
    shader_.setInt("prefilterMap", 7);

    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, brdfLUT_); // BRDF LUT 是 2D 纹理
    shader_.setInt("brdfLUT", 8);

    // 传递相机位置（用于菲涅尔和反射向量）
    shader_.setVec3("camPos", camera.Position);

    // 传递预过滤贴图的最大 LOD 级别
    shader_.setFloat("maxReflectionLOD", MAX_REFLECTION_LOD);


    // 渲染屏幕四边形，执行 IBL 计算
    renderQuad();

    unbindFramebuffer(); // 解绑 FBO
}

void IBLPass::Resize(int width, int height)
{
    // 重新调整输出纹理的大小
    setViewport(width, height);
    // 这里需要重新创建或调整 outputTexture_ 的大小
    // 你需要确保 createColorAttachment 支持重新创建或 updateTextureSize 这样的功能
    // 或者简单粗暴地销毁旧的再创建新的 (通常不推荐频繁操作)
    // 假设 createColorAttachment 内部会处理好
    // outputTexture_ = createColorAttachment(width, height, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT0);
    // 重新绑定到 FBO
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture_, 0);
    // 这部分需要根据你的 RenderPass::createColorAttachment 和 resize 实现来完善。
    // 如果 createColorAttachment 会在每次调用时生成新的纹理，那么你需要自行管理旧纹理的删除。
    // 更常见的做法是有一个公共的 resizeTexture 函数。
    // 为简化，这里假设 createColorAttachment 在 Resize 阶段会被正确调用和处理。
    glDeleteTextures(1, &outputTexture_); // 删除旧的
    outputTexture_ = createColorAttachment(width, height, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT0);
    bindFramebuffer(); // 重新绑定 FBO 以确保更新了纹理
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture_, 0);
    // 检查 FBO 状态
    auto err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (err != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "ERROR::FRAMEBUFFER::IBLPass Framebuffer resize is not complete! Error: " << err << std::endl;
    }
    unbindFramebuffer();
}

// initScreenQuad 和 renderQuad 与 LightPass 中的实现相同
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

    glGenVertexArrays(1, &quadVAO_); // 使用 glGenVertexArrays
    glGenBuffers(1, &quadVBO_);     // 使用 glGenBuffers

    glBindVertexArray(quadVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    // 设置顶点属性指针
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