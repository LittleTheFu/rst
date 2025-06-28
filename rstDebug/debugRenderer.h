#ifndef DEBUG_RENDERER_H
#define DEBUG_RENDERER_H

#include <glad/glad.h>
#include <Eigen/Dense>      // For Eigen::Matrix4f, Eigen::Vector3f
#include <memory>           // For std::unique_ptr
#include "shader.h"         // Your Shader class
#include "DebugBox.h"       // Your DebugBox class
#include "BoundingVolume.h" // Your AABB class (assuming BoundingVolume.h defines AABB)
#include "Camera.h"         // To get view/projection matrices (optional, can be passed directly)
#include "pointLight.h"     // To draw point lights

/**
 * @brief 调试渲染器类。
 * 负责绘制各种调试辅助几何体，例如包围盒、点光源等。
 * 它管理自己的着色器和 DebugBox 实例。
 */
class DebugRenderer {
public:
    /**
     * @brief 构造函数。
     * 初始化 DebugBox 实例。
     */
    DebugRenderer();

    /**
     * @brief 析构函数。
     * unique_ptr 会自动管理着色器资源。
     */
    ~DebugRenderer() = default;

    /**
     * @brief 初始化调试渲染器所使用的着色器。
     * 必须在使用任何 Draw 方法之前调用。
     * @param vertexPath 顶点着色器文件路径。
     * @param fragmentPath 片段着色器文件路径。
     */
    void InitShader(const std::string& vertexPath, const std::string& fragmentPath);

    /**
     * @brief 设置调试渲染器当前使用的视图和投影矩阵。
     * 通常在每次渲染循环开始时从相机获取并设置。
     * @param view 视图矩阵。
     * @param projection 投影矩阵。
     */
    void SetMatrices(const Eigen::Matrix4f& view, const Eigen::Matrix4f& projection);

    /**
     * @brief 绘制一个轴对齐包围盒 (AABB) 的线框。
     * @param aabb 要绘制的 AABB 对象 (世界空间)。
     * @param color 线框的颜色。
     */
    void DrawAABB(const AABB& aabb, const Eigen::Vector3f& color = Eigen::Vector3f(1.0f, 1.0f, 0.0f)) const;

    /**
     * @brief 绘制一个点光源的调试表示 (例如一个小立方体或球体)。
     * @param light 点光源对象。
     * @param size 调试表示的大小。
     * @param color 调试表示的颜色。
     */
    void DrawPointLight(const PointLight& light, float size = 0.2f, const Eigen::Vector3f& color = Eigen::Vector3f(1.0f, 0.0f, 0.0f)) const;

    // TODO: 可以添加更多绘制方法，例如：
    // void DrawSphere(const Eigen::Vector3f& center, float radius, const Eigen::Vector3f& color) const;
    // void DrawLine(const Eigen::Vector3f& start, const Eigen::Vector3f& end, const Eigen::Vector3f& color) const;
    // void DrawRay(const Eigen::Vector3f& origin, const Eigen::Vector3f& direction, float length, const Eigen::Vector3f& color) const;

private:
    std::shared_ptr<Shader> shader_;  // 调试渲染器使用的着色器
    DebugBox debugBox_;               // 用于绘制立方体线框的 DebugBox 实例

    Eigen::Matrix4f viewMatrix_;      // 当前的视图矩阵
    Eigen::Matrix4f projectionMatrix_; // 当前的投影矩阵

    // 辅助函数：保存和恢复 OpenGL 状态
    void SaveGLState() const;
    void RestoreGLState() const;

    // 用于保存/恢复OpenGL状态的成员变量
    mutable GLint originalPolygonMode_[2];
    mutable GLboolean originalCullFaceEnabled_;
    mutable GLboolean originalDepthTestEnabled_;
    mutable GLboolean originalDepthMaskEnabled_;
};

#endif // DEBUG_RENDERER_H