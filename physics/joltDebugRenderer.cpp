#include "joltDebugRenderer.h"
// #include "debug_utils.h" // Included via joltDebugRenderer.h
// #include "shaderManager.h" // Included via joltDebugRenderer.h
#include <iostream>
#include <array>
#include <cstddef> // Required for offsetof
#include <Jolt/Core/Array.h> // For JPH::Array
#include <Jolt/Math/Mat44.h> // For JPH::RMat44Arg conversions
#include <shaderManager.h>

// Constructor: Initializes members to safe values
JoltDebugRenderer::JoltDebugRenderer()
{
    Initialize(); // Call base class's Initialize()
}

// Destructor: Ensures OpenGL resources are cleaned up
JoltDebugRenderer::~JoltDebugRenderer()
{
    Shutdown();
}

void JoltDebugRenderer::Init()
{
    std::cout << "JoltDebugRenderer::Init() - Initializing Jolt Debug Renderer..." << std::endl;

    mShader = ShaderManager::getInstance().loadShader("shader/jolt_debug.vert", "shader/jolt_debug.frag");
    mWireframeShader = ShaderManager::getInstance().loadShader("shader/jolt_debug.vert", "shader/jolt_debug.frag"); // Usually same shader, but different render state

    GL_CHECK_ERROR();
    if (!mShader || !mShader->isValid() || !mWireframeShader || !mWireframeShader->isValid()) {
        std::cerr << "ERROR: JoltDebugRenderer shader initialization failed! Check shader paths." << std::endl;
        return;
    }

    // --- Setup for Triangles (dynamic, accumulated per frame) ---
    glGenVertexArrays(1, &mTriangleVAO);
    glGenBuffers(1, &mTriangleVBO);
    glGenBuffers(1, &mTriangleColorVBO);

    glBindVertexArray(mTriangleVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, mTriangleVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW); 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, mTriangleColorVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW); 
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    // --- Setup for Lines (dynamic, accumulated per frame) ---
    glGenVertexArrays(1, &mLineVAO);
    glGenBuffers(1, &mLineVBO);
    glGenBuffers(1, &mLineColorVBO);

    glBindVertexArray(mLineVAO);

    glBindBuffer(GL_ARRAY_BUFFER, mLineVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW); 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, mLineColorVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW); 
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0); // Unbind VAO
    GL_CHECK_ERROR();
    std::cout << "JoltDebugRenderer::Init() - Jolt Debug Renderer initialized." << std::endl;
}

void JoltDebugRenderer::Shutdown()
{
    std::cout << "JoltDebugRenderer::Shutdown() - Shutting down Jolt Debug Renderer..." << std::endl;
    if (mTriangleVAO != 0) glDeleteVertexArrays(1, &mTriangleVAO);
    if (mTriangleVBO != 0) glDeleteBuffers(1, &mTriangleVBO);
    if (mTriangleColorVBO != 0) glDeleteBuffers(1, &mTriangleColorVBO);

    if (mLineVAO != 0) glDeleteVertexArrays(1, &mLineVAO);
    if (mLineVBO != 0) glDeleteBuffers(1, &mLineVBO);
    if (mLineColorVBO != 0) glDeleteBuffers(1, &mLineColorVBO);

    mShader.reset();
    mWireframeShader.reset();
    GL_CHECK_ERROR();
    std::cout << "JoltDebugRenderer::Shutdown() - Jolt Debug Renderer shutdown complete." << std::endl;
}

void JoltDebugRenderer::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, JPH::DebugRenderer::ECastShadow inCastShadow)
{
    mTriangleVertices.push_back(inV1.GetX()); mTriangleVertices.push_back(inV1.GetY()); mTriangleVertices.push_back(inV1.GetZ());
    mTriangleVertices.push_back(inV2.GetX()); mTriangleVertices.push_back(inV2.GetY()); mTriangleVertices.push_back(inV2.GetZ());
    mTriangleVertices.push_back(inV3.GetX()); mTriangleVertices.push_back(inV3.GetY()); mTriangleVertices.push_back(inV3.GetZ());

    Eigen::Vector4f c = ToEigen(inColor);
    for (int i = 0; i < 3; ++i) {
        mTriangleColors.push_back(c.x()); mTriangleColors.push_back(c.y()); mTriangleColors.push_back(c.z()); mTriangleColors.push_back(c.w());
    }
}

void JoltDebugRenderer::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
{
    mLineVertices.push_back(inFrom.GetX()); mLineVertices.push_back(inFrom.GetY()); mLineVertices.push_back(inFrom.GetZ());
    mLineVertices.push_back(inTo.GetX()); mLineVertices.push_back(inTo.GetY()); mLineVertices.push_back(inTo.GetZ());

    Eigen::Vector4f c = ToEigen(inColor);
    for (int i = 0; i < 2; ++i) {
        mLineColors.push_back(c.x()); mLineColors.push_back(c.y()); mLineColors.push_back(c.z()); mLineColors.push_back(c.w());
    }
}

void JoltDebugRenderer::DrawPoint(JPH::RVec3Arg inP, JPH::ColorArg inColor, float inSize)
{
    float halfSize = inSize * 0.5f;
    DrawLine(inP - JPH::Vec3(halfSize, 0, 0), inP + JPH::Vec3(halfSize, 0, 0), inColor);
    DrawLine(inP - JPH::Vec3(0, halfSize, 0), inP + JPH::Vec3(0, halfSize, 0), inColor);
    DrawLine(inP - JPH::Vec3(0, 0, halfSize), inP + JPH::Vec3(0, 0, halfSize), inColor);
}

JPH::DebugRenderer::Batch JoltDebugRenderer::CreateTriangleBatch(const JPH::DebugRenderer::Triangle *inTriangles, int inTriangleCount)
{
    if (inTriangleCount == 0 || inTriangles == nullptr)
        return nullptr;

    JPH::Array<JPH::DebugRenderer::Vertex> vertices;
    JPH::Array<JPH::uint32> indices;
    vertices.reserve(inTriangleCount * 3);
    indices.reserve(inTriangleCount * 3);

    for (int i = 0; i < inTriangleCount; ++i)
    {
        const JPH::DebugRenderer::Triangle& tri = inTriangles[i];
        for (int j = 0; j < 3; ++j)
        {
            vertices.push_back(tri.mV[j]);
            indices.push_back(i * 3 + j); 
        }
    }

    // 创建 VertexBuffer 和 IndexBuffer
    VertexBuffer vbo(vertices.data(), vertices.size() * sizeof(JPH::DebugRenderer::Vertex), GL_STATIC_DRAW);
    IndexBuffer ibo(indices.data(), indices.size(), GL_STATIC_DRAW);

    // 创建 VertexArray
    VertexArray vao;
    vao.bind(); // 绑定 VAO 以设置属性

    // 设置顶点属性：位置 (Location 0)
    vao.setAttribute(0, vbo, 3, GL_FLOAT, GL_FALSE, offsetof(JPH::DebugRenderer::Vertex, mPosition), sizeof(JPH::DebugRenderer::Vertex), 0);
    vao.enableAttribute(0);

    // 设置顶点属性：颜色 (Location 1)
    vao.setAttribute(1, vbo, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(JPH::DebugRenderer::Vertex, mColor), sizeof(JPH::DebugRenderer::Vertex), 0);
    vao.enableAttribute(1);

    // 设置顶点属性：法线 (Location 2)
    vao.setAttribute(2, vbo, 3, GL_FLOAT, GL_FALSE, offsetof(JPH::DebugRenderer::Vertex, mNormal), sizeof(JPH::DebugRenderer::Vertex), 0);
    vao.enableAttribute(2);
    
    // 设置顶点属性：UV (Location 3)
    vao.setAttribute(3, vbo, 2, GL_FLOAT, GL_FALSE, offsetof(JPH::DebugRenderer::Vertex, mUV), sizeof(JPH::DebugRenderer::Vertex), 0);
    vao.enableAttribute(3);

    // 将 IndexBuffer 绑定到此 VAO
    vao.setIndexBuffer(ibo);

    vao.unbind(); // 解绑 VAO
    GL_CHECK_ERROR();

    // 返回 JoltGLBatch 实例，通过移动语义传递所有权
    return new JoltGLBatch(std::move(vao), std::move(vbo), std::move(ibo));
}

JPH::DebugRenderer::Batch JoltDebugRenderer::CreateTriangleBatch(const JPH::DebugRenderer::Vertex *inVertices, int inVertexCount, const JPH::uint32 *inIndices, int inIndexCount)
{
    if (inVertexCount == 0 || inIndexCount == 0 || inVertices == nullptr || inIndices == nullptr)
        return nullptr;

    // 创建 VertexBuffer 和 IndexBuffer
    VertexBuffer vbo(inVertices, inVertexCount * sizeof(JPH::DebugRenderer::Vertex), GL_STATIC_DRAW);
    IndexBuffer ibo(inIndices, inIndexCount, GL_STATIC_DRAW);

    // 创建 VertexArray
    VertexArray vao;
    vao.bind(); // 绑定 VAO 以设置属性

    // 设置顶点属性：位置 (Location 0)
    vao.setAttribute(0, vbo, 3, GL_FLOAT, GL_FALSE, offsetof(JPH::DebugRenderer::Vertex, mPosition), sizeof(JPH::DebugRenderer::Vertex), 0);
    vao.enableAttribute(0);

    // 设置顶点属性：颜色 (Location 1)
    vao.setAttribute(1, vbo, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(JPH::DebugRenderer::Vertex, mColor), sizeof(JPH::DebugRenderer::Vertex), 0);
    vao.enableAttribute(1);

    // 设置顶点属性：法线 (Location 2)
    vao.setAttribute(2, vbo, 3, GL_FLOAT, GL_FALSE, offsetof(JPH::DebugRenderer::Vertex, mNormal), sizeof(JPH::DebugRenderer::Vertex), 0);
    vao.enableAttribute(2);
    
    // 设置顶点属性：UV (Location 3)
    vao.setAttribute(3, vbo, 2, GL_FLOAT, GL_FALSE, offsetof(JPH::DebugRenderer::Vertex, mUV), sizeof(JPH::DebugRenderer::Vertex), 0);
    vao.enableAttribute(3);

    // 将 IndexBuffer 绑定到此 VAO
    vao.setIndexBuffer(ibo);

    vao.unbind(); // 解绑 VAO
    GL_CHECK_ERROR();

    // 返回 JoltGLBatch 实例，通过移动语义传递所有权
    return new JoltGLBatch(std::move(vao), std::move(vbo), std::move(ibo));
}

void JoltDebugRenderer::DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox &inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const JPH::DebugRenderer::GeometryRef &inGeometry, JPH::DebugRenderer::ECullMode inCullMode, JPH::DebugRenderer::ECastShadow inCastShadow, JPH::DebugRenderer::EDrawMode inDrawMode)
{
    if (!mShader || !mShader->isValid() || !mWireframeShader || !mWireframeShader->isValid()) {
        std::cerr << "WARNING: Jolt Debug Shaders are not initialized or invalid. Skipping DrawGeometry." << std::endl;
        return;
    }

    // Get the appropriate LOD batch from the geometry
    // For simplicity, using sZero for camera position, but in a real app, use your actual camera position
    const JPH::DebugRenderer::LOD& lod = inGeometry->GetLOD(JPH::RVec3::sZero(), inWorldSpaceBounds, inLODScaleSq); 
    
    // Cast the generic Batch to your specific JoltGLBatch type
    JoltGLBatch* batch = static_cast<JoltGLBatch*>(lod.mTriangleBatch.GetPtr());
    if (batch == nullptr || batch->getIndexCount() == 0) {
        return; 
    }

    SaveGLState(); // Save current OpenGL state and set common debug states

    Eigen::Matrix4f modelMatrix;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            modelMatrix(i, j) = (float)inModelMatrix(i, j); 
        }
    }

    Shader* currentShader = (inDrawMode == JPH::DebugRenderer::EDrawMode::Wireframe) ? mWireframeShader.get() : mShader.get();
    currentShader->use();
    currentShader->setMat4("model", modelMatrix); 
    // currentShader->setVec4("uModelColor", ToEigen(inModelColor)); // Pass model color to shader

    if (inDrawMode == JPH::DebugRenderer::EDrawMode::Wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(1.5f); 
    } else { 
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (inCullMode == JPH::DebugRenderer::ECullMode::CullBackFace) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    } else if (inCullMode == JPH::DebugRenderer::ECullMode::CullFrontFace) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
    } else { 
        glDisable(GL_CULL_FACE);
    }

    glEnable(GL_DEPTH_TEST);
    glDepthMask(inDrawMode == JPH::DebugRenderer::EDrawMode::Solid ? GL_TRUE : GL_FALSE);
    
    // 使用你的批处理对象的 bind 方法
    batch->bind();
    glDrawElements(GL_TRIANGLES, batch->getIndexCount(), batch->getIndexType(), nullptr);
    batch->unbind(); // 解绑批处理对象的 VAO
    GL_CHECK_ERROR();

    RestoreGLState();
}

void JoltDebugRenderer::DrawText3D(JPH::RVec3Arg inPosition, const std::string_view &inString, JPH::ColorArg inColor, float inHeight)
{
    // Text rendering is complex and typically requires a dedicated text rendering system.
    // For Jolt debugging, this is often omitted or handled by an overlay UI (e.g., Dear ImGui).
    // std::cout << "JoltDebugRenderer::DrawText3D not implemented for rendering." << std::endl;
}

void JoltDebugRenderer::Clear()
{
    mTriangleVertices.clear();
    mTriangleColors.clear();
    mLineVertices.clear();
    mLineColors.clear();
}

void JoltDebugRenderer::Flush(const Eigen::Matrix4f& view, const Eigen::Matrix4f& projection)
{
    if (!mShader || !mShader->isValid()) {
        std::cerr << "WARNING: Jolt Debug Shader is not initialized or invalid. Skipping debug draw." << std::endl;
        return;
    }

    SaveGLState(); // Save current OpenGL state and set common debug states for Flush

    mShader->use();
    GL_CHECK_ERROR();

    mShader->setMat4("view", view);
    GL_CHECK_ERROR();

    mShader->setMat4("projection", projection);
    GL_CHECK_ERROR();

    mShader->setMat4("model", Eigen::Matrix4f::Identity()); 
    GL_CHECK_ERROR();

    if (!mTriangleVertices.empty())
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDepthMask(GL_FALSE); // Typically don't write depth for accumulated debug lines/triangles if they're overlays
        glBindVertexArray(mTriangleVAO);
        glBindBuffer(GL_ARRAY_BUFFER, mTriangleVBO);
        glBufferData(GL_ARRAY_BUFFER, mTriangleVertices.size() * sizeof(float), mTriangleVertices.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, mTriangleColorVBO);
        glBufferData(GL_ARRAY_BUFFER, mTriangleColors.size() * sizeof(float), mTriangleColors.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)mTriangleVertices.size() / 3);
        GL_CHECK_ERROR();
    }

    if (!mLineVertices.empty())
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        GL_CHECK_ERROR();

        glLineWidth(1.0f);
        GL_CHECK_ERROR();

        glDepthMask(GL_FALSE);
        GL_CHECK_ERROR();

        glBindVertexArray(mLineVAO);
        GL_CHECK_ERROR();

        glBindBuffer(GL_ARRAY_BUFFER, mLineVBO);
        GL_CHECK_ERROR();

        glBufferData(GL_ARRAY_BUFFER, mLineVertices.size() * sizeof(float), mLineVertices.data(), GL_DYNAMIC_DRAW);
        GL_CHECK_ERROR();

        glBindBuffer(GL_ARRAY_BUFFER, mLineColorVBO);
        GL_CHECK_ERROR();

        glBufferData(GL_ARRAY_BUFFER, mLineColors.size() * sizeof(float), mLineColors.data(), GL_DYNAMIC_DRAW);
        GL_CHECK_ERROR();

        glDrawArrays(GL_LINES, 0, (GLsizei)mLineVertices.size() / 3);
        GL_CHECK_ERROR();

        glLineWidth(1.0f);

        GL_CHECK_ERROR();
    }

    glBindVertexArray(0); 
    glUseProgram(0);      
    RestoreGLState();     
    GL_CHECK_ERROR();
}

void JoltDebugRenderer::SaveGLState() const {
    glGetIntegerv(GL_POLYGON_MODE, originalPolygonMode_);
    originalCullFaceEnabled_ = glIsEnabled(GL_CULL_FACE);
    originalDepthTestEnabled_ = glIsEnabled(GL_DEPTH_TEST);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &originalDepthMaskEnabled_);
    originalBlendEnabled_ = glIsEnabled(GL_BLEND);
    glGetIntegerv(GL_BLEND_SRC_RGB, &originalBlendSrcFunc_);
    glGetIntegerv(GL_BLEND_DST_RGB, &originalBlendDstFunc_);

    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    GL_CHECK_ERROR();
}

void JoltDebugRenderer::RestoreGLState() const {
    glPolygonMode(GL_FRONT_AND_BACK, originalPolygonMode_[0]);
    if (originalCullFaceEnabled_) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    if (originalDepthTestEnabled_) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    glDepthMask(originalDepthMaskEnabled_);
    if (originalBlendEnabled_) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    glBlendFunc(originalBlendSrcFunc_, originalBlendDstFunc_);
    GL_CHECK_ERROR();
}