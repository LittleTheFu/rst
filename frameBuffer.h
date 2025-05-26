// Framebuffer.h
#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "GLObject.h" // 继承自 GLObject
#include "GLException.h" // 假设你的 GLException 仍然有用
#include "DepthStencilAttachmentType.h" // 深度/模板附件类型

#include <vector>
#include <string> // For std::to_string

class Framebuffer : public GLObject { // 继承自 GLObject
public:
    /**
     * @brief 构造函数，创建并配置一个帧缓冲。
     * @param width 帧缓冲的宽度。
     * @param height 帧缓冲的高度。
     * @param colorFormats 颜色附件的内部格式列表。vector的大小决定了颜色附件的数量。
     * @param dsType 深度/模板附件的类型（无、纹理或渲染缓冲区）。
     */
    Framebuffer(int width, int height, const std::vector<GLenum>& colorFormats, DepthStencilAttachmentType dsType);

    // 析构函数：不需要显式定义，因为基类 GLObject 的虚析构函数会负责调用 deleteGlResource()。
    // ~Framebuffer() override; // 移除此行，让基类管理

    // --- 核心的渲染目标绑定方法（仍然需要）---
    // 绑定帧缓冲，使其成为当前的渲染目标
    void activate() const;

    // 解绑帧缓冲 (绑定到默认帧缓冲)，停止向此FBO渲染
    void deactivate() const;

    /**
     * @brief 重新调整帧缓冲及其附件的大小。
     * 这是一个复杂的DSA操作，需要重新创建和附加纹理/RBO。
     * @param width 新的宽度。
     * @param height 新的高度。
     */
    void resize(int width, int height);

    // 获取颜色附件纹理ID的列表
    const std::vector<GLuint>& getColorAttachments() const { return colorAttachments_; }

    // 获取指定索引的颜色附件纹理ID
    GLuint getColorAttachment(size_t index = 0) const {
        if (index >= colorAttachments_.size()) {
            throw std::out_of_range("Color attachment index out of range.");
        }
        return colorAttachments_[index];
    }

    // 获取深度/模板附件ID
    GLuint getDepthStencilAttachment() const { return depthStencilAttachment_; }

    // 获取帧缓冲宽度
    int getWidth() const { return width_; }

    // 获取帧缓冲高度
    int getHeight() const { return height_; }

protected:
    // 释放 OpenGL 资源的具体实现，覆盖 GLObject 的纯虚函数
    void deleteGlResource() override; // 对应 GLObject::deleteGlResource()

private:
    std::vector<GLuint> colorAttachments_;          // 存储颜色附件的纹理ID (DSA 创建)
    std::vector<GLenum> colorAttachmentFormats_;    // 存储每个颜色附件的原始格式
    GLuint depthStencilAttachment_ = 0;             // 深度/模板附件的ID（纹理或RBO）
    int width_;                                     // 帧缓冲的当前宽度
    int height_;                                    // 帧缓冲的当前高度
    DepthStencilAttachmentType dsType_;              // 存储深度/模板附件的类型

    // --- 辅助函数：内部实现细节，现在会使用 DSA 函数 ---
    // 创建颜色附件，现在将使用 glCreateTextures 和 glTextureStorage2D
    void createColorAttachments();

    // 创建深度和/或模板附件，现在将使用 glCreateRenderbuffers 或 glCreateTextures
    void createDepthStencilAttachment();

    // 辅助函数：清理旧的附件ID，用于 resize 或析构
    void releaseAttachments();
};

#endif // FRAMEBUFFER_H