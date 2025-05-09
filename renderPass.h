#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <string>
#include <vector>
#include <glad/glad.h> // 需要 GLAD 来使用 OpenGL 函数
#include "Shader.h"    // 假设你已经有 Shader 类
#include "Camera.h"    // 假设你已经有 Camera 类

// 假设你有一个存储场景数据的结构体或类
struct SceneData;

class RenderPass
{
public:
    RenderPass(const std::string &name);
    virtual ~RenderPass() = default;

    // 初始化 Pass，例如创建 Framebuffer 和纹理附件
    virtual void Initialize(int width, int height) = 0;

    // 执行渲染 Pass
    virtual void Render(SceneData &sceneData, Camera &camera) = 0;

    // 处理窗口大小改变
    virtual void Resize(int width, int height);

    // 获取所有颜色附件纹理
    std::vector<GLuint> getColorAttachments() const;

    // 获取指定索引的颜色附件纹理
    GLuint getColorAttachment(size_t index) const;

protected:
    std::string name_;
    GLuint framebuffer_;
    std::vector<GLuint> colorAttachments_;
    GLuint depthAttachment_ = 0;
    Shader shader_;

    // 创建并绑定 Framebuffer
    void createFramebuffer();

    // 创建颜色附件纹理
    GLuint createColorAttachment(int width, int height, GLenum internalFormat, GLenum format, GLenum type, GLenum attachment);

    // 创建深度附件纹理 (Renderbuffer 或 Texture)
    GLuint createDepthAttachment(int width, int height, bool useTexture = false);

    // 绑定 Framebuffer 进行读写
    void bindFramebuffer(bool read = true, bool draw = true);

    // 解绑 Framebuffer，恢复默认 Framebuffer (通常是 0)
    void unbindFramebuffer();

    // 清除指定的缓冲
    void clearBuffers(GLbitfield mask);

    // 设置视口大小
    void setViewport(int width, int height);

    // 启用 OpenGL 状态
    void enableState(GLenum state);

    // 禁用 OpenGL 状态
    void disableState(GLenum state);

    // 获取 Shader 程序 ID
    GLuint getShaderProgram() const;

    // 设置 Shader 的 Uniform 变量 (提供一些常用的重载)
    void setUniformInt(const std::string &name, int value);
    void setUniformFloat(const std::string &name, float value);
    void setUniformVec3(const std::string &name, const Eigen::Vector3f &value);
    void setUniformMat4(const std::string &name, const Eigen::Matrix4f &value);
};

#endif // RENDERPASS_H