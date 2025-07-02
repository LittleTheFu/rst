// RmlUiOpenGLRenderer.h
#ifndef RMLUI_OPENGL_RENDERER_H
#define RMLUI_OPENGL_RENDERER_H

#include <RmlUi/Core/RenderInterface.h>
#include <RmlUi/Core/Core.h>
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
    Eigen::Vector2i dimensions; // 纹理尺寸
};

/**
 * @brief RmlUiOpenGLRenderer 实现了 RmlUi 的 RenderInterface，
 * 使用你的 OpenGL 封装类 (Shader, VertexArray, VertexBuffer, etc.) 进行渲染。
 */
class RmlUiOpenGLRenderer : public Rml::Core::RenderInterface {
public:
    RmlUiOpenGLRenderer();
    virtual ~RmlUiOpenGLRenderer();

    /**
     * @brief 初始化渲染器，加载 RmlUi 渲染所需的着色器。
     * @return 如果初始化成功返回 true，否则返回 false。
     */
    bool Initialize();

    /**
     * @brief 编译并绑定 RmlUi 渲染所需的着色器。
     * 此着色器用于绘制 RmlUi 的几何体，支持纹理和颜色。
     */
    void CompileAndBindShader();

    // ===================================================================================
    // Rml::Core::RenderInterface 接口实现
    // ===================================================================================

    /**
     * @brief 设置当前渲染目标为屏幕或自定义渲染目标。
     * 由于 RmlUi 通常渲染到屏幕，此函数可能不会有太多逻辑。
     * @param target_width 渲染目标的宽度。
     * @param target_height 渲染目标的高度。
     */
    void SetViewport(int target_width, int target_height) override;

    /**
     * @brief 渲染 RmlUi 生成的几何体。
     * @param vertices 顶点数据。
     * @param indices 索引数据。
     * @param texture 纹理句柄，如果为 0 则表示无纹理。
     * @param translation 几何体的平移矩阵。
     */
    void RenderGeometry(Rml::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::Core::TextureHandle texture, const Rml::Core::Vector2f& translation) override;

    /**
     * @brief 启用或禁用裁剪。
     * @param enable 如果为 true 则启用裁剪，否则禁用。
     */
    void EnableScissorRegion(bool enable) override;

    /**
     * @brief 设置裁剪矩形。
     * @param x 裁剪矩形的左上角 X 坐标。
     * @param y 裁剪矩形的左上角 Y 坐标。
     * @param width 裁剪矩形的宽度。
     * @param height 裁剪矩形的高度。
     */
    void SetScissorRegion(int x, int y, int width, int height) override;

    /**
     * @brief 生成一个纹理。
     * @param handle 生成的纹理句柄。
     * @param source 纹理数据源。
     * @param source_dimensions 纹理的尺寸。
     * @return 如果生成成功返回 true，否则返回 false。
     */
    bool LoadTexture(Rml::Core::TextureHandle& texture_handle, Rml::Core::Vector2i& texture_dimensions, const Rml::Core::String& source) override;

    /**
     * @brief 生成一个纹理。
     * @param handle 生成的纹理句柄。
     * @param source 纹理数据源。
     * @param source_dimensions 纹理的尺寸。
     * @return 如果生成成功返回 true，否则返回 false。
     */
    bool GenerateTexture(Rml::Core::TextureHandle& texture_handle, const Rml::Core::byte* source, const Rml::Core::Vector2i& source_dimensions) override;

    /**
     * @brief 释放一个纹理。
     * @param texture 纹理句柄。
     */
    void ReleaseTexture(Rml::Core::TextureHandle texture) override;

    /**
     * @brief 释放所有纹理。
     */
    void ReleaseAllTextures() override;

private:
    // RmlUi 渲染所需的着色器
    std::shared_ptr<Shader> rmlUiShader_;

    // 纹理句柄到实际纹理对象的映射
    std::map<Rml::Core::TextureHandle, RmlUiTextureHandle> textureMap_;
    Rml::Core::TextureHandle nextTextureHandle_ = 1; // 用于生成唯一的纹理句柄

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