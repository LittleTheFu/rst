#include "ShadowUtils.h"
#include <iostream> // 仅用于可能的调试输出

namespace ShadowUtils {

    // 实现一个 Eigen 版本的透视投影矩阵
    // 注意：Eigen 默认是列主序，这个矩阵的构造与 GLM 类似，但直接使用 Eigen 语法
    Eigen::Matrix4f CreatePerspectiveProjectionMatrix(float fovRadians, float aspectRatio, float nearPlane, float farPlane) {
        Eigen::Matrix4f proj = Eigen::Matrix4f::Zero(); // 初始化为零矩阵

        float tanHalfFovy = std::tan(fovRadians / 2.0f);

        // 设置透视投影矩阵的元素
        // 这是标准透视投影矩阵的列主序形式
        proj(0, 0) = 1.0f / (aspectRatio * tanHalfFovy);
        proj(1, 1) = 1.0f / tanHalfFovy;
        proj(2, 2) = -(farPlane + nearPlane) / (farPlane - nearPlane);
        proj(2, 3) = -2.0f * farPlane * nearPlane / (farPlane - nearPlane);
        proj(3, 2) = -1.0f; // 观察空间 Z 映射到裁剪空间 W

        return proj;
    }

    Eigen::Matrix4f CalculateLookAtMatrix(const Eigen::Vector3f& eye, const Eigen::Vector3f& center, const Eigen::Vector3f& up) {
        Eigen::Vector3f f = (center - eye).normalized(); // 前向向量
        Eigen::Vector3f u = up.normalized();            // 标准化的上方向向量

        // 如果 up 和 f 共线，这会产生问题，可以添加一个小的 epsilon 检查
        // 或者在应用程序层面确保 up 和 (center - eye) 不共线
        Eigen::Vector3f s = f.cross(u).normalized();    // 右向量 (叉积 f 和 u)
        u = s.cross(f);                                 // 修正后的上方向向量 (叉积 s 和 f)

        Eigen::Matrix4f result = Eigen::Matrix4f::Identity(); // 初始化为单位矩阵

        // 设置旋转部分
        // 注意 Eigen 默认是列主序，这里我们将向量直接赋值给行
        // result.row(0) = s.transpose(); // 这样也可以，但会创建一个临时矩阵
        result(0,0) = s.x(); result(0,1) = s.y(); result(0,2) = s.z();
        result(1,0) = u.x(); result(1,1) = u.y(); result(1,2) = u.z();
        result(2,0) = -f.x(); result(2,1) = -f.y(); result(2,2) = -f.z(); // 视图矩阵是看向负 Z 轴

        // 设置平移部分
        result(0, 3) = -s.dot(eye);
        result(1, 3) = -u.dot(eye);
        result(2, 3) = f.dot(eye); // 因为视图矩阵是看向负 Z 轴，所以这里是 f.dot(eye)

        return result;
    }

    std::vector<Eigen::Matrix4f> CalculatePointLightSpaceMatrices(
        const Eigen::Vector3f& lightPos,
        float nearPlane,
        float farPlane,
        int shadowMapWidth,
        int shadowMapHeight)
    {
        // 投影矩阵：对于点光源的立方体阴影，FOV 固定为 90 度 (π/2 弧度)
        float fovRadians = M_PI / 2.0f; // 90 度 FOV
        float aspect = (float)shadowMapWidth / (float)shadowMapHeight;
        
        Eigen::Matrix4f shadowProj = CreatePerspectiveProjectionMatrix(fovRadians, aspect, nearPlane, farPlane);

        // 六个方向的视图矩阵目标点和上向量
        std::vector<Eigen::Vector3f> lookTargets = {
            lightPos + Eigen::Vector3f(1.0f, 0.0f, 0.0f),  // +X
            lightPos + Eigen::Vector3f(-1.0f, 0.0f, 0.0f), // -X
            lightPos + Eigen::Vector3f(0.0f, 1.0f, 0.0f),  // +Y
            lightPos + Eigen::Vector3f(0.0f, -1.0f, 0.0f), // -Y
            lightPos + Eigen::Vector3f(0.0f, 0.0f, 1.0f),  // +Z
            lightPos + Eigen::Vector3f(0.0f, 0.0f, -1.0f)  // -Z
        };

        std::vector<Eigen::Vector3f> lookUps = {
            Eigen::Vector3f(0.0f, 1.0f, 0.0f),   // +X 面，上方向是 Y
            Eigen::Vector3f(0.0f, 1.0f, 0.0f),   // -X 面，上方向是 Y
            Eigen::Vector3f(0.0f, 0.0f, 1.0f),   // +Y 面，看向 Y 轴，上方向是 Z 轴 (确保右手坐标系正交)
            Eigen::Vector3f(0.0f, 0.0f, -1.0f),  // -Y 面，看向 -Y 轴，上方向是 -Z 轴
            Eigen::Vector3f(0.0f, 1.0f, 0.0f),   // +Z 面，上方向是 Y
            Eigen::Vector3f(0.0f, 1.0f, 0.0f)    // -Z 面，上方向是 Y
        };

        std::vector<Eigen::Matrix4f> lightSpaceMatrices;
        lightSpaceMatrices.reserve(6); // 预留空间，避免不必要的重新分配

        for (int i = 0; i < 6; ++i) {
            Eigen::Matrix4f lightView = CalculateLookAtMatrix(lightPos, lookTargets[i], lookUps[i]);
            lightSpaceMatrices.push_back(shadowProj * lightView);
        }

        return lightSpaceMatrices;
    }

} // namespace ShadowUtils