#ifndef JOLT_DEBUG_RENDERER_H
#define JOLT_DEBUG_RENDERER_H

// #define JPH_DEBUG_RENDERER (1) // 这一行通常由 CMakeLists.txt 或项目设置控制
#include <Jolt/Jolt.h> // Jolt 的主头文件，包含核心类型

#include <Jolt/Renderer/DebugRenderer.h> // Jolt 的调试渲染器接口

// 引入你自己的 OpenGL 封装类
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "shader.h" // 你的 Shader 类
#include "debug_utils.h" // For GL_CHECK_ERROR()

#include <glad/glad.h>
#include <Eigen/Dense>
#include <vector>
#include <memory>
#include <string_view>
#include <cstddef>     // For offsetof

// ====================================================================
// START: JoltGLBatch - 封装你的 OpenGL 资源管理类
// ====================================================================

// JoltGLBatch 将持有 VertexArray, VertexBuffer, IndexBuffer 的实例。
// 由于你的 GLResource 体系已经处理了资源的创建和释放（通过移动语义和析构函数），
// 这里的 JoltGLBatch 只需要拥有这些对象即可。
class JoltGLBatch : public JPH::RefTargetVirtual
{
public:
    // 构造函数：接受 move 进来的 VertexArray, VertexBuffer, IndexBuffer
    // 这确保了资源的唯一所有权从临时对象转移到这个批处理对象
    JoltGLBatch(VertexArray&& va, VertexBuffer&& vb, IndexBuffer&& ib)
        : mVertexArray(std::move(va)), mVertexBuffer(std::move(vb)), mIndexBuffer(std::move(ib))
    {
    }

    // --- 核心修正：实现 JPH::RefTargetVirtual 的 Release() 方法 ---
    // 这个方法会在 Jolt 的引用计数降为 0 时被调用，负责销毁对象本身。
    virtual void Release() override
    {
        // 直接删除 'this' 指针。
        // 这会触发 JoltGLBatch 的析构函数，
        // 进而调用其成员变量 mVertexArray, mVertexBuffer, mIndexBuffer 的析构函数，
        // 从而正确释放底层的 OpenGL 资源。
        delete this;
    }

    void AddRef() override
    {
        //TODO: later....
    }

    // 析构函数：由 JPH::Ref 智能指针在引用计数归零时调用。
    // 其成员变量 (mVertexArray, mVertexBuffer, mIndexBuffer) 会自动调用它们的析构函数，
    // 从而正确释放 OpenGL 资源 (通过 GLResource 的 release() 机制)。
    // 因此，这里不需要额外的 glDelete* 调用。
    ~JoltGLBatch() = default;

    // 获取索引数量，用于 glDrawElements 调用
    int getIndexCount() const { return static_cast<int>(mIndexBuffer.getCount()); }
    // 获取索引类型，用于 glDrawElements 调用
    GLenum getIndexType() const { return mIndexBuffer.getType(); }

    // 暴露 VertexArray 的 bind/unbind 方法，方便 DrawGeometry 调用
    void bind() const { mVertexArray.bind(); }
    void unbind() const { mVertexArray.unbind(); }

private:
    VertexArray  mVertexArray;
    VertexBuffer mVertexBuffer;
    IndexBuffer  mIndexBuffer;

    // 禁用复制，只允许移动
    JoltGLBatch(const JoltGLBatch&) = delete;
    JoltGLBatch& operator=(const JoltGLBatch&) = delete;
};

// ====================================================================
// END: JoltGLBatch
// ====================================================================


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

    virtual void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, JPH::DebugRenderer::ECastShadow inCastShadow = JPH::DebugRenderer::ECastShadow::Off) override;
    virtual void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;
    void DrawPoint(JPH::RVec3Arg inP, JPH::ColorArg inColor, float inSize);
    
    virtual void DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox &inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const JPH::DebugRenderer::GeometryRef &inGeometry, JPH::DebugRenderer::ECullMode inCullMode = JPH::DebugRenderer::ECullMode::CullBackFace, JPH::DebugRenderer::ECastShadow inCastShadow = JPH::DebugRenderer::ECastShadow::On, JPH::DebugRenderer::EDrawMode inDrawMode = JPH::DebugRenderer::EDrawMode::Solid) override;
    
    virtual void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view &inString, JPH::ColorArg inColor = JPH::Color::sWhite, float inHeight = 0.5f) override;

    // --- 实现缺失的纯虚函数 ---
    // 这些函数创建并返回一个三角形批次，用于高效渲染
    virtual JPH::DebugRenderer::Batch CreateTriangleBatch(const JPH::DebugRenderer::Triangle *inTriangles, int inTriangleCount) override;
    virtual JPH::DebugRenderer::Batch CreateTriangleBatch(const JPH::DebugRenderer::Vertex *inVertices, int inVertexCount, const JPH::uint32 *inIndices, int inIndexCount) override;

    // --- 自定义管理函数 ---
    void Clear();
    void Flush(const Eigen::Matrix4f& view, const Eigen::Matrix4f& projection);

private:
    std::vector<float> mTriangleVertices;
    std::vector<float> mTriangleColors;
    std::vector<float> mLineVertices;
    std::vector<float> mLineColors;

    GLuint mTriangleVAO = 0;
    GLuint mTriangleVBO = 0;
    GLuint mTriangleColorVBO = 0;

    GLuint mLineVAO = 0;
    GLuint mLineVBO = 0;
    GLuint mLineColorVBO = 0;

    std::shared_ptr<Shader> mShader;
    std::shared_ptr<Shader> mWireframeShader;

    mutable GLint originalPolygonMode_[2];
    mutable GLboolean originalCullFaceEnabled_;
    mutable GLboolean originalDepthTestEnabled_;
    mutable GLboolean originalDepthMaskEnabled_;
    mutable GLboolean originalBlendEnabled_;
    mutable GLint originalBlendSrcFunc_;
    mutable GLint originalBlendDstFunc_;

    inline Eigen::Vector3f ToEigen(const JPH::Vec3& v) { return Eigen::Vector3f(v.GetX(), v.GetY(), v.GetZ()); }
    inline Eigen::Vector4f ToEigen(const JPH::Color& c) { return Eigen::Vector4f(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f); }

    void SaveGLState() const;
    void RestoreGLState() const;
};

#endif // JOLT_DEBUG_RENDERER_H