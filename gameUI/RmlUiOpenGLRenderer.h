// RmlUiOpenGLRenderer.h (更新后，使用兼容性适配器)
#ifndef RMLUI_OPENGL_RENDERER_H
#define RMLUI_OPENGL_RENDERER_H

// 引入兼容性适配器头文件
#include <RmlUi/Core/RenderInterfaceCompatibility.h>
#include <RmlUi/Core/Core.h> // 仍然需要 Core.h 来获取 Rml::Vertex, Rml::Vector2i, Rml::TextureHandle 等
#include <RmlUi/Core/FileInterface.h> // For texture loading paths

#include <glad/glad.h>
#include <Eigen/Dense> // For Eigen::Matrix4f

#include <map>
#include <memory> // For std::shared_ptr

// 引入你的 OpenGL 封装类
#include "Shader.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Texture2D.h"
#include "TextureManager.h" // 用于加载纹理

// 定义 RmlUi 纹理句柄的结构
struct RmlUiTextureHandle {
    std::shared_ptr<Texture2D> texture;
    Rml::Vector2i dimensions; // 纹理尺寸 (使用 Rml::Vector2i)
};

/**
 * @brief RmlUiOpenGLRenderer 实现了 RmlUi 的 RenderInterface，
 * 使用你的 OpenGL 封装类 (Shader, VertexArray, VertexBuffer, etc.) 进行渲染。
 * 此版本使用 RenderInterfaceCompatibility 适配器。
 */
class RmlUiOpenGLRenderer : public Rml::RenderInterfaceCompatibility { // <--- 继承 RenderInterfaceCompatibility
public:
    RmlUiOpenGLRenderer();
    virtual ~RmlUiOpenGLRenderer();

    /**
     * @brief 初始化渲染器，加载 RmlUi 渲染所需的着色器。
     * @return 如果初始化成功返回 true，否则返回 false。
     */
    bool Initialize();

    // ===================================================================================
    // Rml::RenderInterfaceCompatibility 接口实现 (旧 RenderInterface 的签名)
    // ===================================================================================

    /**
     * @brief 设置当前渲染目标为屏幕或自定义渲染目标。
     * @param target_width 渲染目标的宽度。
     * @param target_height 渲染目标的高度。
     */
    void SetViewport(int target_width, int target_height); // <--- 移除 override 关键字

    /**
     * @brief 渲染 RmlUi 生成的几何体。
     * @param vertices 顶点数据。
     * @param indices 索引数据。
     * @param texture 纹理句柄，如果为 0 则表示无纹理。
     * @param translation 几何体的平移矩阵。
     */
    void RenderGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture, const Rml::Vector2f& translation); // <--- 移除 override 关键字，修正 Rml:: 到 Rml::

    /**
     * @brief 启用或禁用裁剪。
     * @param enable 如果为 true 则启用裁剪，否则禁用。
     */
    void EnableScissorRegion(bool enable); // <--- 移除 override 关键字

    /**
     * @brief 设置裁剪矩形。
     * @param x 裁剪矩形的左上角 X 坐标。
     * @param y 裁剪矩形的左上角 Y 坐标。
     * @param width 裁剪矩形的宽度。
     * @param height 裁剪矩形的高度。
     */
    void SetScissorRegion(int x, int y, int width, int height); // <--- 移除 override 关键字

    /**
     * @brief 生成一个纹理。
     * @param handle 生成的纹理句柄。
     * @param source 纹理数据源。
     * @param source_dimensions 纹理的尺寸。
     * @return 如果生成成功返回 true，否则返回 false。
     */
    bool LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source); // <--- 移除 override 关键字，修正 Rml:: 到 Rml::

    /**
     * @brief 生成一个纹理。
     * @param handle 生成的纹理句柄。
     * @param source 纹理数据源。
     * @param source_dimensions 纹理的尺寸。
     * @return 如果生成成功返回 true，否则返回 false。
     */
    bool GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions); // <--- 移除 override 关键字，修正 Rml:: 到 Rml::

    /**
     * @brief 释放一个纹理。
     * @param texture 纹理句柄。
     */
    void ReleaseTexture(Rml::TextureHandle texture); // <--- 移除 override 关键字，修正 Rml:: 到 Rml::

    /**
     * @brief 释放所有纹理。
     */
    void ReleaseAllTextures(); // <--- 移除 override 关键字

private:
    // RmlUi 渲染所需的着色器
    std::shared_ptr<Shader> rmlUiShader_;

    // 纹理句柄到实际纹理对象的映射
    std::map<Rml::TextureHandle, RmlUiTextureHandle> textureMap_; // <--- 修正 Rml:: 到 Rml::
    Rml::TextureHandle nextTextureHandle_ = 1; // 用于生成唯一的纹理句柄 (修正 Rml:: 到 Rml::)

    // 投影矩阵
    Eigen::Matrix4f projectionMatrix_;

    // 渲染状态
    GLint lastProgramId_;
    GLboolean lastBlendEnabled_;
    GLboolean lastScissorEnabled_;
    GLint lastScissorBox_[4];
    GLboolean lastCullFaceEnabled_;
    GLboolean lastDepthTestEnabled_;
    GLint lastActiveTexture_;
    GLint lastTextureBinding2D_;
    GLint lastVertexArrayBinding_;
    GLint lastViewport_[4];
};

#endif // RMLUI_OPENGL_RENDERER_H
