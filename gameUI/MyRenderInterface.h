#ifndef MY_RENDER_INTERFACE_H
#define MY_RENDER_INTERFACE_H

#include <glad/glad.h>
#include <memory>
#include <string>

// 引入您已有的类
#include "shader.h"
#include "texture2D.h"
#include "textureCubeMap.h"
#include "vertexArray.h"
#include "vertexBuffer.h"
#include "indexBuffer.h"
#include "debug_utils.h" // 用于 GL_CHECK_ERROR
#include "glException.h" // 用于 THROW_GL_EXCEPTION

// 假设您使用 Eigen 作为数学库
#include <Eigen/Dense>

/**
 * @brief MyRenderInterface 是一个单例类，提供高级的渲染操作接口。
 * 它封装了底层的 OpenGL 调用，并利用您自定义的资源管理类。
 */
class MyRenderInterface {
public:
    /**
     * @brief 获取 MyRenderInterface 的唯一实例。
     * @return MyRenderInterface 实例的引用。
     */
    static MyRenderInterface& getInstance();

    // --- 初始化与清空 ---

    /**
     * @brief 初始化渲染器。通常在 OpenGL 上下文创建后调用。
     * 在此处可以设置一些初始的 OpenGL 状态。
     */
    void initialize();

    /**
     * @brief 设置清除颜色。
     * @param r 红色分量 (0.0 - 1.0)。
     * @param g 绿色分量 (0.0 - 1.0)。
     * @param b 蓝色分量 (0.0 - 1.0)。
     * @param a Alpha 分量 (0.0 - 1.0)。
     */
    void setClearColor(float r, float g, float b, float a);

    /**
     * @brief 清除颜色缓冲区和深度缓冲区。
     */
    void clearBuffers();

    /**
     * @brief 设置渲染视口。
     * @param x 视口左下角的 X 坐标。
     * @param y 视口左下角的 Y 坐标。
     * @param width 视口宽度。
     * @param height 视口高度。
     */
    void setViewport(int x, int y, int width, int height);

    // --- 状态管理 ---

    /**
     * @brief 启用 OpenGL 能力。
     * @param capability 要启用的能力 (例如 GL_DEPTH_TEST, GL_BLEND)。
     */
    void enable(GLenum capability);

    /**
     * @brief 禁用 OpenGL 能力。
     * @param capability 要禁用的能力。
     */
    void disable(GLenum capability);

    /**
     * @brief 启用深度测试并设置深度函数。
     * @param func 深度函数 (例如 GL_LESS, GL_LEQUAL)。
     */
    void enableDepthTest(GLenum func = GL_LESS);

    /**
     * @brief 禁用深度测试。
     */
    void disableDepthTest();

    /**
     * @brief 启用混合并设置混合函数。
     * @param sfactor 源因子 (例如 GL_SRC_ALPHA)。
     * @param dfactor 目标因子 (例如 GL_ONE_MINUS_SRC_ALPHA)。
     */
    void enableBlending(GLenum sfactor = GL_SRC_ALPHA, GLenum dfactor = GL_ONE_MINUS_SRC_ALPHA);

    /**
     * @brief 禁用混合。
     */
    void disableBlending();

    /**
     * @brief 启用背面剔除。
     * @param mode 剔除模式 (GL_BACK, GL_FRONT, GL_FRONT_AND_BACK)。
     */
    void enableCulling(GLenum mode = GL_BACK);

    /**
     * @brief 禁用剔除。
     */
    void disableCulling();

    /**
     * @brief 设置多边形模式（线框或填充）。
     * @param face 多边形面 (GL_FRONT_AND_BACK)。
     * @param mode 模式 (GL_FILL, GL_LINE, GL_POINT)。
     */
    void setPolygonMode(GLenum face, GLenum mode);

    // --- 绘制命令 ---

    /**
     * @brief 使用索引缓冲区绘制网格。
     * @param vao 顶点数组对象。
     * @param shader 着色器程序。
     * @param count 索引数量。
     * @param type 索引类型 (GL_UNSIGNED_INT, GL_UNSIGNED_SHORT, GL_UNSIGNED_BYTE)。
     * @param mode 绘制模式 (例如 GL_TRIANGLES, GL_LINES)。
     */
    void drawIndexed(const VertexArray& vao, const std::shared_ptr<Shader>& shader,
                     GLsizei count, GLenum type, GLenum mode = GL_TRIANGLES);

    /**
     * @brief 绘制非索引网格。
     * @param vao 顶点数组对象。
     * @param shader 着色器程序。
     * @param vertexCount 顶点数量。
     * @param mode 绘制模式 (例如 GL_TRIANGLES, GL_LINES)。
     * @param first 起始顶点索引。
     */
    void drawArrays(const VertexArray& vao, const std::shared_ptr<Shader>& shader,
                    GLsizei vertexCount, GLenum mode = GL_TRIANGLES, GLint first = 0);

    /**
     * @brief 实例化绘制索引网格。
     * @param vao 顶点数组对象。
     * @param shader 着色器程序。
     * @param count 索引数量。
     * @param type 索引类型 (GL_UNSIGNED_INT, GL_UNSIGNED_SHORT, GL_UNSIGNED_BYTE)。
     * @param instanceCount 实例数量。
     * @param mode 绘制模式 (例如 GL_TRIANGLES, GL_LINES)。
     */
    void drawIndexedInstanced(const VertexArray& vao, const std::shared_ptr<Shader>& shader,
                              GLsizei count, GLenum type, GLsizei instanceCount, GLenum mode = GL_TRIANGLES);

    // --- 着色器与纹理操作 ---

    /**
     * @brief 激活并使用指定的着色器程序。
     * @param shader 要使用的着色器程序的共享指针。
     */
    void useShader(const std::shared_ptr<Shader>& shader);

    /**
     * @brief 绑定 2D 纹理到指定的纹理单元。
     * @param texture 2D 纹理的共享指针。
     * @param textureUnit 纹理单元 (例如 GL_TEXTURE0)。
     */
    void bindTexture(const std::shared_ptr<Texture2D>& texture, GLenum textureUnit);

    /**
     * @brief 绑定立方体贴图纹理到指定的纹理单元。
     * @param texture 立方体贴图纹理的共享指针。
     * @param textureUnit 纹理单元 (例如 GL_TEXTURE0)。
     */
    void bindTexture(const std::shared_ptr<TextureCubeMap>& texture, GLenum textureUnit);

    /**
     * @brief 设置着色器中的一个布尔 uniform 变量。
     * @param shader 着色器程序。
     * @param name uniform 变量的名称。
     * @param value 布尔值。
     */
    void setShaderUniform(const std::shared_ptr<Shader>& shader, const std::string& name, bool value);

    /**
     * @brief 设置着色器中的一个整数 uniform 变量。
     * @param shader 着色器程序。
     * @param name uniform 变量的名称。
     * @param value 整数值。
     */
    void setShaderUniform(const std::shared_ptr<Shader>& shader, const std::string& name, int value);

    /**
     * @brief 设置着色器中的一个浮点 uniform 变量。
     * @param shader 着色器程序。
     * @param name uniform 变量的名称。
     * @param value 浮点值。
     */
    void setShaderUniform(const std::shared_ptr<Shader>& shader, const std::string& name, float value);

    /**
     * @brief 设置着色器中的一个 Vec2 uniform 变量。
     * @param shader 着色器程序。
     * @param name uniform 变量的名称。
     * @param value Vec2 值。
     */
    void setShaderUniform(const std::shared_ptr<Shader>& shader, const std::string& name, const Eigen::Vector2f& value);

    /**
     * @brief 设置着色器中的一个 Vec3 uniform 变量。
     * @param shader 着色器程序。
     * @param name uniform 变量的名称。
     * @param value Vec3 值。
     */
    void setShaderUniform(const std::shared_ptr<Shader>& shader, const std::string& name, const Eigen::Vector3f& value);

    /**
     * @brief 设置着色器中的一个 Vec4 uniform 变量。
     * @param shader 着色器程序。
     * @param name uniform 变量的名称。
     * @param value Vec4 值。
     */
    void setShaderUniform(const std::shared_ptr<Shader>& shader, const std::string& name, const Eigen::Vector4f& value);

    /**
     * @brief 设置着色器中的一个 Mat4 uniform 变量。
     * @param shader 着色器程序。
     * @param name uniform 变量的名称。
     * @param value Mat4 值。
     */
    void setShaderUniform(const std::shared_ptr<Shader>& shader, const std::string& name, const Eigen::Matrix4f& value);

private:
    // 单例模式的私有成员
    MyRenderInterface() = default;
    ~MyRenderInterface() = default;
    MyRenderInterface(const MyRenderInterface&) = delete;
    MyRenderInterface& operator=(const MyRenderInterface&) = delete;
};

#endif // MY_RENDER_INTERFACE_H