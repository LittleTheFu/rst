#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <string>
#include <vector>
#include <glad/glad.h> // 需要 GLAD 来使用 OpenGL 函数
#include <memory>      // 用于 std::unique_ptr

// 确保 Framebuffer 头文件已包含
#include "Framebuffer.h"

// 假设你有通用的 SceneData 和 Camera 类型，但 RenderPass 基类不再直接依赖它们
// struct SceneData;
// class Camera;

class RenderPass
{
public:
    // 构造函数：初始化 Pass 的名称、尺寸，并创建基础的 Framebuffer 容器
    // 此时 Framebuffer 只是一个空容器，没有附件
    RenderPass(const std::string &name, int width, int height);

    // 虚析构函数，确保派生类正确析构
    virtual ~RenderPass() = default;

    // 执行渲染 Pass 的纯虚函数。
    // 具体渲染所需的输入数据将通过派生类的构造函数或 setter 注入，作为其成员持有。
    virtual void Render() = 0;

    // 处理窗口大小改变。默认实现会更新 Pass 的尺寸，
    // 但附件的 Resize 逻辑需要由派生类负责实现。
    virtual void Resize(int width, int height);

    // 获取 Pass 的名称
    const std::string& getName() const { return name_; }

protected:
    // 每个 RenderPass 都拥有一个 Framebuffer 实例作为其渲染目标
    std::unique_ptr<Framebuffer> frameBuffer_;
    int width_;  // Pass 的渲染目标宽度
    int height_; // Pass 的渲染目标高度

    std::string name_; // Pass 的名称

    // --- 辅助方法 (封装 OpenGL 通用操作，供派生类使用) ---

    // 激活此 Pass 的 Framebuffer，使其成为当前绘制目标
    void activateFramebuffer() const;

    // 解激活此 Pass 的 Framebuffer，恢复默认 Framebuffer (通常是屏幕)
    void deactivateFramebuffer() const;

    // 清除指定的缓冲 (例如 GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    void clearBuffers(GLbitfield mask);

    // 设置视口大小 (通常设置为 Pass 的宽度和高度)
    void setViewport(int width, int height) const;

    // 启用 OpenGL 状态 (例如 GL_DEPTH_TEST, GL_CULL_FACE)
    void enableState(GLenum state);

    // 禁用 OpenGL 状态
    void disableState(GLenum state);
};

#endif // RENDERPASS_H