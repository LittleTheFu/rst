// #include "skyPass.h"
// #include "camera.h" // 确保包含 camera 头文件
// #include <iostream>
// #include <vector>
// #include "debug_utils.h" // 确保包含调试工具

// // 立方体顶点数据，用于渲染天空盒
// float skyboxVertices[] = {
//     // positions
//     -1.0f,  1.0f, -1.0f,
//     -1.0f, -1.0f, -1.0f,
//      1.0f, -1.0f, -1.0f,
//      1.0f, -1.0f, -1.0f,
//      1.0f,  1.0f, -1.0f,
//     -1.0f,  1.0f, -1.0f,

//     -1.0f, -1.0f,  1.0f,
//     -1.0f, -1.0f, -1.0f,
//     -1.0f,  1.0f, -1.0f,
//     -1.0f,  1.0f, -1.0f,
//     -1.0f,  1.0f,  1.0f,
//     -1.0f, -1.0f,  1.0f,

//      1.0f, -1.0f, -1.0f,
//      1.0f, -1.0f,  1.0f,
//      1.0f,  1.0f,  1.0f,
//      1.0f,  1.0f,  1.0f,
//      1.0f,  1.0f, -1.0f,
//      1.0f, -1.0f, -1.0f,

//     -1.0f, -1.0f,  1.0f,
//     -1.0f,  1.0f,  1.0f,
//      1.0f,  1.0f,  1.0f,
//      1.0f,  1.0f,  1.0f,
//      1.0f, -1.0f,  1.0f,
//     -1.0f, -1.0f,  1.0f,

//     -1.0f,  1.0f, -1.0f,
//      1.0f,  1.0f, -1.0f,
//      1.0f,  1.0f,  1.0f,
//      1.0f,  1.0f,  1.0f,
//     -1.0f,  1.0f,  1.0f,
//     -1.0f,  1.0f, -1.0f,

//     -1.0f, -1.0f, -1.0f,
//     -1.0f, -1.0f,  1.0f,
//      1.0f, -1.0f, -1.0f,
//      1.0f, -1.0f, -1.0f,
//      1.0f, -1.0f,  1.0f,
//     -1.0f, -1.0f,  1.0f
// };

// SkyPass::SkyPass(int width, int height, std::shared_ptr<TextureCubeMap> skyboxTexture)
//     : RenderPass("SkyPass", width, height), // 调用基类构造函数
//       skyboxTexture_(skyboxTexture) // 初始化天空盒纹理智能指针
// {
//     shader_.load("shader/skybox.vert", "shader/skybox.frag"); // 假设天空盒 Shader 文件名为 skybox.vert 和 skybox.frag

//     // 1. 创建 Framebuffer
//     frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);

//     // 2. 创建颜色附件纹理
//     auto colorTex = std::make_unique<Texture2D>(width_, height_, GL_RGBA16F, 1); // 通常天空盒输出为 HDR
//     colorTexture_ = colorTex->id(); // 获取其 OpenGL ID
//     frameBuffer_->attachColorTexture(colorTexture_, GL_COLOR_ATTACHMENT0, 0);

//     // 3. 创建深度渲染缓冲对象 (Renderbuffer)
//     glCreateRenderbuffers(1, &depthRBO_);
//     glNamedRenderbufferStorage(depthRBO_, GL_DEPTH_COMPONENT, width_, height_);
//     frameBuffer_->attachDepthRenderbuffer(depthRBO_);

//     // 4. 设置绘制缓冲区
//     std::vector<GLenum> drawBuffers = {GL_COLOR_ATTACHMENT0};
//     frameBuffer_->setDrawBuffers(drawBuffers);

//     // 5. 检查 Framebuffer 完整性
//     frameBuffer_->checkCompleteness();

//     initSkyboxCube(); // 初始化天空盒立方体 VAO/VBO
// }

// // 实现基类的纯虚函数 Render()，不带参数
// void SkyPass::Render()
// {
//     if (!skyboxTexture_) {
//         std::cerr << "Warning: Skybox texture not set for SkyPass!" << std::endl;
//         return;
//     }

//     // 1. 绑定此 Pass 的 Framebuffer
//     activateFramebuffer();
//     setViewport(width_, height_);

//     // 2. 清除缓冲
//     glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//     clearBuffers(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//     // 3. 启用深度测试但禁用深度写入
//     // 天空盒应该在所有其他物体之后渲染，并且不写入深度缓冲，这样其他物体才能正确显示
//     enableState(GL_DEPTH_TEST);
//     glDepthFunc(GL_LEQUAL); // 改变深度函数，让深度值等于或小于深度缓冲中的值时通过测试 (因为 z 值为 1.0 的天空盒顶点在远平面)
//     glDepthMask(GL_FALSE);  // 禁用深度写入！

//     // 4. 使用天空盒 Shader
//     shader_.use();

//     // 5. 设置 Uniform 变量
//     // 对于天空盒，我们只需要 View 矩阵的旋转部分（移除平移）
//     Eigen::Matrix4f view = camera_.GetViewMatrix();
//     Eigen::Matrix4f projection = camera_.GetProjectionMatrix();

//     shader_.setMat4("view", view);
//     shader_.setMat4("projection", projection);

//     // 6. 绑定天空盒纹理
//     skyboxTexture_->activate(GL_TEXTURE0); // 绑定到纹理单元 0
//     shader_.setInt("skybox", 0);

//     // 7. 渲染天空盒立方体
//     renderSkyboxCube();

//     // 8. 恢复 OpenGL 状态
//     glDepthMask(GL_TRUE);   // 恢复深度写入
//     glDepthFunc(GL_LESS);   // 恢复默认深度函数
//     // disableState(GL_DEPTH_TEST); // 根据后续 Pass 是否需要深度测试来决定是否禁用

//     // 9. 解绑此 Pass 的 Framebuffer
//     deactivateFramebuffer();
// }

// void SkyPass::Resize(int width, int height)
// {
//     // 如果尺寸没有变化，则无需重新创建
//     if (width == width_ && height == height_) {
//         return;
//     }

//     // 更新基类的尺寸
//     RenderPass::Resize(width, height);

//     // 重新创建 Framebuffer 和附件
//     deactivateFramebuffer(); // 首先解绑 FBO

//     // 删除旧的纹理和 RBO
//     if (colorTexture_ != 0) {
//         glDeleteTextures(1, &colorTexture_);
//         colorTexture_ = 0;
//     }
//     if (depthRBO_ != 0) {
//         glDeleteRenderbuffers(1, &depthRBO_);
//         depthRBO_ = 0;
//     }

//     // 重新创建 Framebuffer 对象 (这将生成新的 ID 并释放旧的)
//     frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);

//     // 重新创建并附加颜色输出纹理
//     auto colorTex = std::make_unique<Texture2D>(width_, height_, GL_RGBA16F, 1);
//     colorTexture_ = colorTex->id();
//     frameBuffer_->attachColorTexture(colorTexture_, GL_COLOR_ATTACHMENT0, 0);

//     // 重新创建并附加深度渲染缓冲对象
//     glCreateRenderbuffers(1, &depthRBO_);
//     glNamedRenderbufferStorage(depthRBO_, GL_DEPTH_COMPONENT, width_, height_);
//     frameBuffer_->attachDepthRenderbuffer(depthRBO_);

//     // 重新设置绘制缓冲区
//     std::vector<GLenum> drawBuffers = {GL_COLOR_ATTACHMENT0};
//     frameBuffer_->setDrawBuffers(drawBuffers);

//     // 检查 FBO 完整性
//     frameBuffer_->checkCompleteness();
// }

// void SkyPass::initSkyboxCube()
// {
//     glCreateVertexArrays(1, &skyboxVAO_);
//     glBindVertexArray(skyboxVAO_);

//     glCreateBuffers(1, &skyboxVBO_);
//     glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO_);
//     glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

//     // 设置顶点属性指针 (只有位置)
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
//     glEnableVertexAttribArray(0);

//     glBindVertexArray(0);
//     GL_CHECK_ERROR();
// }

// void SkyPass::renderSkyboxCube()
// {
//     glBindVertexArray(skyboxVAO_);
//     glDrawArrays(GL_TRIANGLES, 0, 36); // 立方体有 36 个顶点
//     glBindVertexArray(0);
//     GL_CHECK_ERROR();
// }