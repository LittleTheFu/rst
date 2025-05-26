#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <string>
#include <vector>
#include <glad/glad.h>   // 需要 GLAD 来使用 OpenGL 函数
#include "Camera.h"   // 假设你已经有 Camera 类
#include "Framebuffer.h" // 包含 Framebuffer 类，现在它只管理 FBO 本身

// 假设你有一个存储场景数据的结构体或类
struct SceneData; // 保持不变

class RenderPass
{
public:
    // 构造函数：初始化 Pass 的名称和尺寸，但不创建 Framebuffer 附件
    RenderPass(const std::string &name, int width, int height);
    virtual ~RenderPass() = default;

    // 执行渲染 Pass (纯虚函数，由派生类实现)
    virtual void Render(SceneData &sceneData, Camera &camera) = 0;

    // 处理窗口大小改变。默认实现可以为空，派生类可以根据需要重写
    virtual void Resize(int width, int height);

protected:
    std::unique_ptr<Framebuffer> frameBuffer_; // Framebuffer 实例，由派生类负责配置
    int width_;                                // Pass 的渲染目标宽度
    int height_;                               // Pass 的渲染目标高度

    std::string name_; // Pass 的名称

    // --- 辅助方法 (封装 OpenGL 通用操作) ---
    void activateFramebuffer() const;

    // 解激活此 Pass 的 Framebuffer。现在它直接调用 frameBuffer_->deactivate();
    void deactivateFramebuffer() const;

    // 清除指定的缓冲
    void clearBuffers(GLbitfield mask);

    // 设置视口大小
    void setViewport(int width, int height);

    // 启用 OpenGL 状态
    void enableState(GLenum state);

    // 禁用 OpenGL 状态
    void disableState(GLenum state);
};

#endif // RENDERPASS_H