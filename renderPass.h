#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <string>
#include <vector>
#include <glad/glad.h>
#include <memory> // For std::unique_ptr

#include "Camera.h"
#include "Framebuffer.h" // 包含 Framebuffer 类

struct SceneData; // 假设你有一个存储场景数据的结构体或类

class RenderPass
{
public:
    // 构造函数：初始化 Pass 的名称和尺寸，并创建基础的 Framebuffer 容器
    RenderPass(const std::string &name, int width, int height);
    virtual ~RenderPass() = default;

    // 执行渲染 Pass (纯虚函数，由派生类实现具体的渲染逻辑)
    virtual void Render(SceneData &sceneData, Camera &camera) = 0;

    // 处理窗口大小改变。默认实现会更新 Pass 的尺寸，但附件的 Resize 逻辑由派生类负责。
    virtual void Resize(int width, int height);

protected:
    // 每个 RenderPass 都拥有一个 Framebuffer 实例作为其渲染目标
    std::unique_ptr<Framebuffer> frameBuffer_;
    int width_;
    int height_;

    std::string name_; // Pass 的名称

    // --- 辅助方法 (封装 OpenGL 通用操作) ---

    // 激活此 Pass 的 Framebuffer
    void activateFramebuffer() const;

    // 解激活此 Pass 的 Framebuffer
    void deactivateFramebuffer() const;

    // 清除指定的缓冲
    void clearBuffers(GLbitfield mask);

    // 设置视口大小 (通常设置为 Pass 的宽度和高度)
    void setViewport(int width, int height) const;

    // 启用 OpenGL 状态
    void enableState(GLenum state);

    // 禁用 OpenGL 状态
    void disableState(GLenum state);
};

#endif // RENDERPASS_H