#include "skyPass.h"
#include "camera.h" // 确保包含 camera 头文件
#include <iostream>
#include <vector>
#include "debug_utils.h" // 确保包含调试工具


SkyPass::SkyPass(int width, int height, std::shared_ptr<TextureCubeMap> skyboxTexture)
    : RenderPass("SkyPass", width, height),
      skyboxTexture_(skyboxTexture)
{
    shader_.load("shader/skybox.vert", "shader/skybox.frag");
    initFrameBuffer();
}

void SkyPass::initFrameBuffer()
{
    frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);

    // 1. 创建颜色附件纹理
    colorTexture_ = std::make_unique<Texture2D>(width_, height_, GL_RGBA8);
    frameBuffer_->attachColorTexture(colorTexture_->id(), GL_COLOR_ATTACHMENT0, 0); // 假设颜色附件为 0

    // 2. 创建深度渲染缓冲对象 (Renderbuffer)
    depthTexture_ = std::make_unique<Texture2D>(width_, height_, GL_DEPTH_COMPONENT24);
    frameBuffer_->attachDepthTexture(depthTexture_->id()); // 直接使用纹理 ID 作为深度附件

    std::vector<GLenum> drawBuffers = {GL_COLOR_ATTACHMENT0};

    frameBuffer_->checkCompleteness();

    GL_CHECK_ERROR(); // 检查 OpenGL 错误
}

// 实现基类的纯虚函数 Render()，不带参数
void SkyPass::Render(const Camera &camera)
{
    if (!skyboxTexture_) {
        assert(0);
        std::cerr << "Warning: Skybox texture not set for SkyPass!" << std::endl;
        return;
    }

    // 1. 绑定此 Pass 的 Framebuffer
    activateFramebuffer();
    setViewport(width_, height_);

    // 2. 清除缓冲
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    clearBuffers(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // enableState(GL_DEPTH_TEST);
    // glDepthFunc(GL_LEQUAL); // 改变深度函数，让深度值等于或小于深度缓冲中的值时通过测试 (因为 z 值为 1.0 的天空盒顶点在远平面)
    // glDepthMask(GL_FALSE);  // 禁用深度写入！

    // 4. 使用天空盒 Shader
    shader_.use();

    // // 提取旋转部分：保留左上 3x3，构造新的 4x4 矩阵
    Eigen::Matrix4f view = camera.GetViewMatrix();
    Eigen::Matrix4f rotationOnly = Eigen::Matrix4f::Identity();
    rotationOnly.block<3,3>(0,0) = view.block<3,3>(0,0); // 复制旋转部分

    // 5. 设置 Uniform 变量
    // 对于天空盒，我们只需要 View 矩阵的旋转部分（移除平移）
    Eigen::Matrix4f projection = camera.GetProjectionMatrix();

    shader_.setMat4("view", rotationOnly);
    shader_.setMat4("projection", projection);

    // 6. 绑定天空盒纹理
    skyboxTexture_->activate(0); // 绑定到纹理单元 0
    shader_.setInt("skybox", 0);

    skyboxCube_.render();

    // 8. 恢复 OpenGL 状态
    // glDepthMask(GL_TRUE);   // 恢复深度写入
    // glDepthFunc(GL_LESS);   // 恢复默认深度函数
    // disableState(GL_DEPTH_TEST); // 根据后续 Pass 是否需要深度测试来决定是否禁用

    // 9. 解绑此 Pass 的 Framebuffer
    deactivateFramebuffer();
}

void SkyPass::Resize(int width, int height)
{
    // 如果尺寸没有变化，则无需重新创建
    if (width == width_ && height == height_) {
        return;
    }

    // 更新基类的尺寸
    RenderPass::Resize(width, height);

    // 重新创建 Framebuffer 和附件
    deactivateFramebuffer(); // 首先解绑 FBO

    initFrameBuffer();
}