#include "joltDebugRenderer.h"
#include "debug_utils.h" // For GL_CHECK_ERROR()
#include "shaderManager.h" // To load the shader using your ShaderManager
#include <iostream>
#include <array> // 需要引入 <array>，因为 Jolt 的一些内部操作可能需要

// Constructor: Initializes members to safe values
JoltDebugRenderer::JoltDebugRenderer()
{
    // !!! IMPORTANT !!! 你必须在这里调用基类的 Initialize()
    // 这是 Jolt 要求其 DebugRenderer 实现必须做的
    Initialize(); 
}

// Destructor: Ensures OpenGL resources are cleaned up
JoltDebugRenderer::~JoltDebugRenderer()
{
    Shutdown();
}

void JoltDebugRenderer::Init()
{
    std::cout << "JoltDebugRenderer::Init() - Initializing Jolt Debug Renderer..." << std::endl;

    // Load the debug shader using your ShaderManager
    mShader = ShaderManager::getInstance().loadShader("shader/jolt_debug.vert", "shader/jolt_debug.frag");
    mWireframeShader = ShaderManager::getInstance().loadShader("shader/jolt_debug_wireframe.vert", "shader/jolt_debug_wireframe.frag"); // 新增：加载线框着色器

    GL_CHECK_ERROR();
    if (!mShader || !mShader->isValid() || !mWireframeShader || !mWireframeShader->isValid()) {
        std::cerr << "ERROR: JoltDebugRenderer shader initialization failed! Check shader paths." << std::endl;
        // Handle error: perhaps set a flag that debug rendering is unavailable
        return;
    }

    // --- Setup for Triangles ---
    glGenVertexArrays(1, &mTriangleVAO);
    glGenBuffers(1, &mTriangleVBO);
    glGenBuffers(1, &mTriangleColorVBO);

    glBindVertexArray(mTriangleVAO);
    
    // Vertex positions (Location 0)
    glBindBuffer(GL_ARRAY_BUFFER, mTriangleVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW); 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Vertex colors (Location 1)
    glBindBuffer(GL_ARRAY_BUFFER, mTriangleColorVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW); 
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    // --- Setup for Lines ---
    glGenVertexArrays(1, &mLineVAO);
    glGenBuffers(1, &mLineVBO);
    glGenBuffers(1, &mLineColorVBO);

    glBindVertexArray(mLineVAO);

    // Line positions (Location 0)
    glBindBuffer(GL_ARRAY_BUFFER, mLineVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW); 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Line colors (Location 1)
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
    // Delete OpenGL objects if they were generated
    if (mTriangleVAO != 0) glDeleteVertexArrays(1, &mTriangleVAO);
    if (mTriangleVBO != 0) glDeleteBuffers(1, &mTriangleVBO);
    if (mTriangleColorVBO != 0) glDeleteBuffers(1, &mTriangleColorVBO);

    if (mLineVAO != 0) glDeleteVertexArrays(1, &mLineVAO);
    if (mLineVBO != 0) glDeleteBuffers(1, &mLineVBO);
    if (mLineColorVBO != 0) glDeleteBuffers(1, &mLineColorVBO);

    // Shader unique_ptr will handle deletion
    mShader.reset();
    mWireframeShader.reset(); // 新增：释放线框着色器
    GL_CHECK_ERROR();
    std::cout << "JoltDebugRenderer::Shutdown() - Jolt Debug Renderer shutdown complete." << std::endl;
}

// 签名修改：使用 JPH::RVec3Arg, JPH::ColorArg 并添加 JPH::DebugRenderer::ECastShadow 参数
void JoltDebugRenderer::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, JPH::DebugRenderer::ECastShadow inCastShadow)
{
    // Collect vertices
    mTriangleVertices.push_back(inV1.GetX()); mTriangleVertices.push_back(inV1.GetY()); mTriangleVertices.push_back(inV1.GetZ());
    mTriangleVertices.push_back(inV2.GetX()); mTriangleVertices.push_back(inV2.GetY()); mTriangleVertices.push_back(inV2.GetZ());
    mTriangleVertices.push_back(inV3.GetX()); mTriangleVertices.push_back(inV3.GetY()); mTriangleVertices.push_back(inV3.GetZ());

    // Collect colors (same color for all 3 vertices of the triangle)
    Eigen::Vector4f c = ToEigen(inColor);
    for (int i = 0; i < 3; ++i) {
        mTriangleColors.push_back(c.x()); mTriangleColors.push_back(c.y()); mTriangleColors.push_back(c.z()); mTriangleColors.push_back(c.w());
    }
}

// 签名修改：使用 JPH::RVec3Arg, JPH::ColorArg
void JoltDebugRenderer::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
{
    // Collect vertices
    mLineVertices.push_back(inFrom.GetX()); mLineVertices.push_back(inFrom.GetY()); mLineVertices.push_back(inFrom.GetZ());
    mLineVertices.push_back(inTo.GetX()); mLineVertices.push_back(inTo.GetY()); mLineVertices.push_back(inTo.GetZ());

    // Collect colors (same color for both vertices of the line)
    Eigen::Vector4f c = ToEigen(inColor);
    for (int i = 0; i < 2; ++i) {
        mLineColors.push_back(c.x()); mLineColors.push_back(c.y()); mLineColors.push_back(c.z()); mLineColors.push_back(c.w());
    }
}

// 签名修改：移除 override，使其成为普通成员函数
void JoltDebugRenderer::DrawPoint(JPH::RVec3Arg inP, JPH::ColorArg inColor, float inSize)
{
    // For simplicity, we won't draw points explicitly for now.
    // Jolt typically uses lines/triangles for complex shapes' wireframes anyway.
    // If needed, you could render a small cube/sphere at this point using your DebugBox.
    // std::cout << "JoltDebugRenderer::DrawPoint not implemented for rendering." << std::endl;
    // 可以将其转换为 DrawMarker 内部调用 DrawLine 绘制一个小十字
    float halfSize = inSize * 0.5f;
    DrawLine(inP - JPH::Vec3(halfSize, 0, 0), inP + JPH::Vec3(halfSize, 0, 0), inColor);
    DrawLine(inP - JPH::Vec3(0, halfSize, 0), inP + JPH::Vec3(0, halfSize, 0), inColor);
    DrawLine(inP - JPH::Vec3(0, 0, halfSize), inP + JPH::Vec3(0, 0, halfSize), inColor);
}

// 签名修改：使用 JPH::RMat44Arg, JPH::ColorArg，并添加正确的枚举类型和默认值
void JoltDebugRenderer::DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox &inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const JPH::DebugRenderer::GeometryRef &inGeometry, JPH::DebugRenderer::ECullMode inCullMode, JPH::DebugRenderer::ECastShadow inCastShadow, JPH::DebugRenderer::EDrawMode inDrawMode)
{
    // 这个函数是 Jolt 绘制预生成几何体批次的地方。
    // 你需要在这里实现 OpenGL 渲染批次（如 VAO/VBO）的逻辑。
    // 如果你还没有实现 CreateTriangleBatch，那么 inGeometry 的 Batch 成员将为空。
    // 为了临时调试，这里可以调用 DrawTriangle 或 DrawLine 来绘制简单的几何体，
    // 但更高效的方法是按照我之前给的方案实现 CreateTriangleBatch 和 DrawGeometry 的完整逻辑。

    // 为了避免编译错误并临时让项目跑起来，这里可以保持为空，或者打个日志。
    // 但是请注意，如果 DrawGeometry 不处理，Jolt 的一些复杂形状可能不会显示。
    // Jolt 通常会将复杂形状分解为线和三角形，然后调用 DrawTriangle/DrawLine。
    // 所以，目前你的 DrawTriangle 和 DrawLine 实现能够覆盖一部分调试需求。

    // 如果要实现这个函数，大致的框架会是这样（需要你实现 OpenGLTriangleBatch 类和 CreateTriangleBatch 方法）：
    /*
    if (!mShader || !mShader->isValid() || !mWireframeShader || !mWireframeShader->isValid()) {
        std::cerr << "WARNING: Jolt Debug Shaders are not initialized or invalid. Skipping DrawGeometry." << std::endl;
        return;
    }

    const JPH::DebugRenderer::LOD& lod = inGeometry->GetLOD(JPH::DebugRenderer::sInstance->GetCameraPosition(), inWorldSpaceBounds, inLODScaleSq);
    
    // 获取你的自定义 OpenGL 批处理对象
    OpenGLTriangleBatch* batch = static_cast<OpenGLTriangleBatch*>(lod.mTriangleBatch.GetPtr());
    if (batch == nullptr)
        return; 

    SaveGLState();

    Eigen::Vector4f eigenModelColor = ToEigen(inModelColor);
    
    if (inDrawMode == JPH::DebugRenderer::EDrawMode::Wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        mWireframeShader->use();
        mWireframeShader->setVec4("uWireframeColor", eigenModelColor); 
    }
    else // Solid
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        mShader->use();
        mShader->setVec4("uModelColor", eigenModelColor); 
    }

    Shader* currentShader = (inDrawMode == JPH::DebugRenderer::EDrawMode::Wireframe) ? mWireframeShader.get() : mShader.get();
    currentShader->setMat4("model", *reinterpret_cast<const Eigen::Matrix4f*>(&inModelMatrix));
    // View and Projection matrices assumed to be set globally or passed
    
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
    glDepthMask(GL_TRUE); 

    glBindVertexArray(batch->mVAO);

    // This part depends on whether your batch is indexed or not.
    // If you implement OpenGLIndexedTriangleBatch as suggested before:
    // OpenGLIndexedTriangleBatch* indexedBatch = dynamic_cast<OpenGLIndexedTriangleBatch*>(batch);
    // if (indexedBatch && indexedBatch->mEBO != 0) {
    //     glDrawElements(GL_TRIANGLES, indexedBatch->mIndexCount, GL_UNSIGNED_INT, nullptr);
    // } else {
    //     // Handle non-indexed draw (needs vertex count, which isn't stored in base OpenGLTriangleBatch)
    //     // For now, this is a placeholder. Most complex shapes in Jolt are indexed.
    // }

    glBindVertexArray(0);
    GL_CHECK_ERROR();

    RestoreGLState();
    */
}

// 签名修改：使用 JPH::RVec3Arg, JPH::ColorArg 并添加默认值
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

    SaveGLState(); // Save current OpenGL state and set common debug states

    // Ensure face culling is disabled for debug rendering as shapes might not be manifold
    glDisable(GL_CULL_FACE);
    // Enable blending for transparent debug shapes
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Allow debug geometry to be drawn on top, but don't write to depth buffer
    glDepthMask(GL_FALSE);
    GL_CHECK_ERROR();

    // Use the main shader for both triangles and lines for simplicity with this setup
    mShader->use();
    mShader->setMat4("view", view);
    mShader->setMat4("projection", projection);
    mShader->setMat4("model", Eigen::Matrix4f::Identity()); // Debug geometry is usually in world space
    // For this simple shader, we might need a default color if vertex colors aren't used in shader.
    // Assuming your shader uses vertex colors (location 1) as fColor.
    GL_CHECK_ERROR();

    // --- Render Triangles ---
    if (!mTriangleVertices.empty())
    {
        glBindVertexArray(mTriangleVAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, mTriangleVBO);
        glBufferData(GL_ARRAY_BUFFER, mTriangleVertices.size() * sizeof(float), mTriangleVertices.data(), GL_DYNAMIC_DRAW);
        
        glBindBuffer(GL_ARRAY_BUFFER, mTriangleColorVBO);
        glBufferData(GL_ARRAY_BUFFER, mTriangleColors.size() * sizeof(float), mTriangleColors.data(), GL_DYNAMIC_DRAW);
        
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)mTriangleVertices.size() / 3);
        GL_CHECK_ERROR();
    }

    // --- Render Lines ---
    // If you want different shader for lines, you'd need to switch here
    // For now, using the same mShader as you provided
    if (!mLineVertices.empty())
    {
        glBindVertexArray(mLineVAO);

        glBindBuffer(GL_ARRAY_BUFFER, mLineVBO);
        glBufferData(GL_ARRAY_BUFFER, mLineVertices.size() * sizeof(float), mLineVertices.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, mLineColorVBO);
        glBufferData(GL_ARRAY_BUFFER, mLineColors.size() * sizeof(float), mLineColors.data(), GL_DYNAMIC_DRAW);

        glDrawArrays(GL_LINES, 0, (GLsizei)mLineVertices.size() / 3);
        GL_CHECK_ERROR();
    }

    glBindVertexArray(0); // Unbind VAO
    RestoreGLState(); // Restore original OpenGL state
    GL_CHECK_ERROR();
}

void JoltDebugRenderer::SaveGLState() const {
    // 保存 GL_BLEND 状态及因子
    originalBlendEnabled_ = glIsEnabled(GL_BLEND);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &originalBlendSrcFunc_);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &originalBlendDstFunc_);

    // 保存其他状态
    glGetIntegerv(GL_POLYGON_MODE, originalPolygonMode_);
    originalCullFaceEnabled_ = glIsEnabled(GL_CULL_FACE);
    originalDepthTestEnabled_ = glIsEnabled(GL_DEPTH_TEST);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &originalDepthMaskEnabled_);

    // 为调试渲染设置状态
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // For solid triangles, GL_LINES for lines in Flush
    glDisable(GL_CULL_FACE); // Debug rendering often needs both sides
    glEnable(GL_DEPTH_TEST); // Enable depth test to ensure overlay is correct
    glDepthMask(GL_FALSE);   // Disable depth write to draw on top of existing scene
    glEnable(GL_BLEND);      // Ensure blending is on for transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Standard alpha blending
    GL_CHECK_ERROR();
}

void JoltDebugRenderer::RestoreGLState() const {
    // 恢复 GL_BLEND 状态及因子
    if (originalBlendEnabled_) {
        glEnable(GL_BLEND);
        glBlendFunc(originalBlendSrcFunc_, originalBlendDstFunc_);
    } else {
        glDisable(GL_BLEND);
    }

    // 恢复其他状态
    glPolygonMode(GL_FRONT_AND_BACK, originalPolygonMode_[0]);

    if (originalCullFaceEnabled_) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }

    if (originalDepthTestEnabled_) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }

    glDepthMask(originalDepthMaskEnabled_);
    GL_CHECK_ERROR();
}