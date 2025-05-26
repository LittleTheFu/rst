// Framebuffer.h
#pragma once

#include "GLObject.h" // 确保 GLObject 基类可用
#include <glad/glad.h>
#include <vector>
#include <stdexcept> // 用于异常处理


class Framebuffer : public GLObject {
private:
    GLuint id_ = 0; // FBO 的 OpenGL ID
    int width_;      // FBO 的宽度 (主要用于完整性检查和视口设置)
    int height_;     // FBO 的高度 (主要用于完整性检查和视口设置)

public:
    // 构造函数：只创建 FBO 本身，不创建任何附件
    // width 和 height 用于后续的完整性检查，确保附件尺寸匹配 FBO 尺寸
    Framebuffer(int width, int height);

    // 析构函数：由于继承 GLObject，其资源释放逻辑已在基类中处理
    // 但是这里需要确保如果 FBO 还有附件，它们被正确解绑或在外部被释放
    // (通常由外部持有纹理/Renderbuffer智能指针来管理)
    ~Framebuffer() override = default;

    // 激活 FBO，使其成为当前的渲染目标
    void activate() const;

    // 解除 FBO 激活，恢复默认帧缓冲 (屏幕)
    void deactivate() const;

    // --- 附件管理方法 (通过传入纹理/Renderbuffer ID) ---

    // 附加颜色纹理
    // textureID: 要附加的纹理 ID
    // attachmentPoint: 颜色附件点 (如 GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1)
    // mipLevel: 要附加的纹理的 Mipmap 级别
    void attachColorTexture(GLuint textureID, GLenum attachmentPoint, int mipLevel = 0);

    // 附加深度纹理
    // textureID: 要附加的深度纹理 ID
    // mipLevel: 要附加的纹理的 Mipmap 级别
    void attachDepthTexture(GLuint textureID, int mipLevel = 0);

    // 附加深度/模板纹理 (如果纹理同时包含深度和模板)
    // textureID: 要附加的深度/模板纹理 ID
    // mipLevel: 要附加的纹理的 Mipmap 级别
    void attachDepthStencilTexture(GLuint textureID, int mipLevel = 0);

    // 附加深度 Renderbuffer
    // renderbufferID: 要附加的 Renderbuffer ID
    void attachDepthRenderbuffer(GLuint renderbufferID);

    // 附加深度/模板 Renderbuffer (如果 Renderbuffer 同时包含深度和模板)
    // renderbufferID: 要附加的 Renderbuffer ID
    void attachDepthStencilRenderbuffer(GLuint renderbufferID);


    // 设置绘制缓冲区列表 (告诉 FBO 渲染时哪些颜色附件是活跃的)
    // drawBuffers: 包含 GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 等的列表
    void setDrawBuffers(const std::vector<GLenum>& drawBuffers);

    // 检查 FBO 的完整性，确保它能被渲染
    void checkCompleteness() const;

    // 获取 FBO 的 OpenGL ID
    GLuint id() const { return id_; }

    // 获取 FBO 的尺寸
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

protected:
    // 实现 GLObject 的纯虚函数，用于释放 FBO 自身的 OpenGL 资源
    void deleteGlResource() override;
};