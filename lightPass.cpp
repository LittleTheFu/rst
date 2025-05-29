#include "lightPass.h"
#include <iostream>
#include <vector> // 确保包含 vector
#include "pointLightDataForUBO.h" // 确保包含 PointLightDataForUBO
#include "debug_utils.h"

LightPass::LightPass(int width, int height)
    : RenderPass("LightPass", width, height)
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

void LightPass::Render(GLuint gPositionID, GLuint gNormalID, GLuint gAlbedoID,
                       GLuint gRoughnessID, GLuint gMetallicID, GLuint gAOID,
                       const PointLight& light, const Camera& camera,
                       GLuint shadowMapID)
{
    // 1. 绑定 Light Pass 的 Framebuffer
    activateFramebuffer();
    setViewport(width_, height_);

    // 2. 清除颜色缓冲（深度缓冲通常由 G-Buffer 决定或不在此清除）
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    clearBuffers(GL_COLOR_BUFFER_BIT);

    // 3. 禁用深度测试（因为我们已经有了 G-Buffer 的深度，并且在这里渲染全屏四边形）
    disableState(GL_DEPTH_TEST);

    // 4. 绑定 Light Shader
    shader_.use();

    // 5. 绑定 G-Buffer 纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPositionID);
    shader_.setInt("gPosition", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormalID);
    shader_.setInt("gNormal", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gAlbedoID);
    shader_.setInt("gAlbedo", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gRoughnessID);
    shader_.setInt("gRoughness", 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, gMetallicID);
    shader_.setInt("gMetallic", 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, gAOID);
    shader_.setInt("gAO", 5);

    // 6. 绑定阴影贴图 (立方体贴图)
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMapID);
    shader_.setInt("shadowMap", 6);

    // 7. 设置 Uniform 变量
    shader_.setVec3("viewPos", camera.Position);
    shader_.setVec3("light.position", light.position);
    shader_.setVec3("light.color", light.color);
    shader_.setFloat("light.intensity", light.intensity);
    shader_.setFloat("far_plane", 100.0f); // 应该与 ShadowPass 中的 far_plane 一致


    // 8. 渲染全屏四边形
    renderQuad(); // 假设你有一个 renderQuad() 辅助函数来绘制全屏四边形

    // 9. 解绑纹理
    // ... 实际应用中可以省略解绑，因为下一个绘制命令会重新绑定 ...

    // 10. 解绑 Light Pass 的 Framebuffer
    deactivateFramebuffer();

    GL_CHECK_ERROR();
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