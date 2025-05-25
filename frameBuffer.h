#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "GLResource.h"
#include "GLException.h"
#include "DepthStencilAttachmentType.h"
#include <vector>
#include <string> // For std::to_string

class Framebuffer : public GLResource {
public:
    /**
     * @brief 构造函数，创建并配置一个帧缓冲。
     * @param width 帧缓冲的宽度。
     * @param height 帧缓冲的高度。
     * @param colorFormats 颜色附件的内部格式列表。vector的大小决定了颜色附件的数量。
     * @param dsType 深度/模板附件的类型（无、纹理或渲染缓冲区）。
     */
    Framebuffer(int width, int height, const std::vector<GLenum>& colorFormats, DepthStencilAttachmentType dsType);

    // 析构函数，确保资源被正确释放
    ~Framebuffer() override;

    // 绑定帧缓冲
    void bind() const override;

    // 解绑帧缓冲 (绑定到默认帧缓冲)
    void unbind() const override;

    /**
     * @brief 重新调整帧缓冲及其附件的大小。
     * @param width 新的宽度。
     * @param height 新的高度。
     */
    void resize(int width, int height);

    const std::vector<GLuint>& getColorAttachments() const { return colorAttachments_; }

    // 获取颜色附件纹理ID (如果只有一个颜色附件)
    // 如果有多个颜色附件，建议使用 getColorAttachmentID(index)
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
    // 释放 OpenGL 资源的具体实现
    void release() override;

private:
    std::vector<GLuint> colorAttachments_;          // 存储颜色附件的纹理ID
    std::vector<GLenum> colorAttachmentFormats_;    // 存储每个颜色附件的原始格式
    GLuint depthStencilAttachment_ = 0;             // 深度/模板附件的ID（纹理或RBO）
    int width_;                                     // 帧缓冲的当前宽度
    int height_;                                    // 帧缓冲的当前高度
    DepthStencilAttachmentType dsType_;             // 存储深度/模板附件的类型

    /**
     * @brief 辅助函数：创建颜色附件。
     * @param formats 颜色附件的内部格式列表。
     */
    void createColorAttachments();

    /**
     * @brief 辅助函数：创建深度和/或模板附件。
     * @param dsType 深度/模板附件的类型。
     */
    void createDepthStencilAttachment();
};

#endif // FRAMEBUFFER_H