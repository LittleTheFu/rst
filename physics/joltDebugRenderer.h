#ifndef JOLT_DEBUG_RENDERER_H
#define JOLT_DEBUG_RENDERER_H

// #define JPH_DEBUG_RENDERER (1) // 这一行应该由 CMakeLists.txt 控制，所以在这里注释掉
#include <Jolt/Jolt.h> // 这是 Jolt 的主头文件，通常会包含 Core.h, Math/Vec3.h, Core/Color.h 等基础类型

#include <Jolt/Renderer/DebugRenderer.h> // Jolt's base debug renderer interface

// 你自己的头文件
#include <glad/glad.h>
#include <Eigen/Dense>
#include <vector>
#include <memory>
#include <string_view> // 明确包含 string_view
#include "shader.h" // Your Shader class

/**
 * @brief Implements JPH::DebugRenderer to visualize Jolt physics bodies.
 *
 * This class collects vertices and colors from Jolt's physics system
 * and renders them using OpenGL. It specifically implements Jolt's
 * virtual drawing functions like DrawTriangle and DrawLine.
 */
class JoltDebugRenderer : public JPH::DebugRenderer
{
public:
    JoltDebugRenderer();
    ~JoltDebugRenderer();

    /**
     * @brief Initializes the debug renderer, setting up OpenGL VAOs/VBOs
     * and loading the debug shader.
     */
    void Init();

    /**
     * @brief Shuts down the renderer, releasing OpenGL resources.
     */
    void Shutdown();

    // --- JPH::DebugRenderer Interface Implementations ---

    /**
     * @brief Called by Jolt to draw a single triangle.
     * Vertices and color are accumulated internally.
     */
    // 签名修改：使用 RVec3Arg, ColorArg 并添加 ECastShadow 参数及默认值
    virtual void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, JPH::DebugRenderer::ECastShadow inCastShadow = JPH::DebugRenderer::ECastShadow::Off) override;

    /**
     * @brief Called by Jolt to draw a single line.
     * Vertices and color are accumulated internally.
     */
    // 签名修改：使用 RVec3Arg, ColorArg
    virtual void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;

    /**
     * @brief Called by Jolt to draw a point. (Simplified/Ignored for now)
     * NOTE: JPH::DebugRenderer does NOT have a virtual DrawPoint method.
     * We remove 'override' and make it a regular member function if you wish to use it.
     * Jolt will typically call DrawMarker (a non-virtual helper) which then calls DrawLine/DrawTriangle.
     */
    // 移除 override 关键字，因为基类没有这个虚函数
    // 如果 Jolt 不会直接调用这个，你可以保留它作为你自己的辅助函数
    void DrawPoint(JPH::RVec3Arg inP, JPH::ColorArg inColor, float inSize); // 改变参数类型以匹配 Jolt 风格

    /**
     * @brief Called by Jolt to draw complex geometry (like collision shapes).
     * This function often delegates to DrawTriangle/DrawLine for basic rendering.
     */
    // 签名修改：使用 RMat44Arg, ColorArg，并添加正确的枚举类型和默认值
    // GeometryRef 也需要 JPH::DebugRenderer:: 前缀
    virtual void DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox &inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const JPH::DebugRenderer::GeometryRef &inGeometry, JPH::DebugRenderer::ECullMode inCullMode = JPH::DebugRenderer::ECullMode::CullBackFace, JPH::DebugRenderer::ECastShadow inCastShadow = JPH::DebugRenderer::ECastShadow::On, JPH::DebugRenderer::EDrawMode inDrawMode = JPH::DebugRenderer::EDrawMode::Solid) override;
    
    /**
     * @brief Called by Jolt to draw 3D text. (Simplified/Ignored for now)
     */
    // 签名修改：使用 RVec3Arg, ColorArg，并添加默认值
    virtual void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view &inString, JPH::ColorArg inColor = JPH::Color::sWhite, float inHeight = 0.5f) override;

    // --- Custom Management Functions ---

    /**
     * @brief Clears all accumulated vertex data, preparing for a new frame.
     */
    void Clear(); // 通常 JoltDebugRenderer 命名为 ClearTransientPrimitives 更合适

    /**
     * @brief Renders all accumulated debug geometry.
     * @param view The current view matrix from the camera.
     * @param projection The current projection matrix from the camera.
     */
    void Flush(const Eigen::Matrix4f& view, const Eigen::Matrix4f& projection);

private:
    // Buffers to store collected triangle data
    std::vector<float> mTriangleVertices;
    std::vector<float> mTriangleColors;

    // Buffers to store collected line data
    std::vector<float> mLineVertices;
    std::vector<float> mLineColors;

    // OpenGL Vertex Array Objects and Vertex Buffer Objects for triangles
    GLuint mTriangleVAO = 0;
    GLuint mTriangleVBO = 0;
    GLuint mTriangleColorVBO = 0;

    // OpenGL Vertex Array Objects and Vertex Buffer Objects for lines
    GLuint mLineVAO = 0;
    GLuint mLineVBO = 0;
    GLuint mLineColorVBO = 0;

    std::shared_ptr<Shader> mShader; // Shader for debug rendering (uses your Shader class)
    std::shared_ptr<Shader> mWireframeShader; // 新增：用于线框渲染的着色器

    // Cached OpenGL state to restore after rendering debug visuals
    mutable GLint originalPolygonMode_[2];
    mutable GLboolean originalCullFaceEnabled_;
    mutable GLboolean originalDepthTestEnabled_;
    mutable GLboolean originalDepthMaskEnabled_;
    mutable GLboolean originalBlendEnabled_; // 新增：保存和恢复 GL_BLEND 状态
    mutable GLint originalBlendSrcFunc_;    // 新增：保存和恢复混合源因子
    mutable GLint originalBlendDstFunc_;    // 新增：保存和恢复混合目标因子

    // Helper functions for Jolt to Eigen conversion
    // 注意：这里 ToEigen 中的 JPH::Vec3 和 JPH::Color 仍然需要 JPH:: 前缀
    inline Eigen::Vector3f ToEigen(const JPH::Vec3& v) { return Eigen::Vector3f(v.GetX(), v.GetY(), v.GetZ()); }
    inline Eigen::Vector4f ToEigen(const JPH::Color& c) { return Eigen::Vector4f(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f); }

    // Utility functions for saving/restoring GL state
    void SaveGLState() const;
    void RestoreGLState() const;
};

#endif // JOLT_DEBUG_RENDERER_H