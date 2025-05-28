#ifndef SHADOW_UTILS_H
#define SHADOW_UTILS_H

#include <Eigen/Dense>
#include <vector>
#include <cmath> // 用于 std::tan 和 M_PI

namespace ShadowUtils {

    /**
     * @brief 计算一个通用的 LookAt 视图矩阵。
     * 此函数不依赖于任何相机实例的状态，是一个纯粹的数学工具。
     * @param eye 相机/观察点的位置。
     * @param center 观察点看向的目标位置。
     * @param up 世界空间的上方向向量。
     * @return 计算出的视图矩阵。
     */
    Eigen::Matrix4f CalculateLookAtMatrix(const Eigen::Vector3f& eye, const Eigen::Vector3f& center, const Eigen::Vector3f& up);

    /**
     * @brief 为点光源阴影贴图生成六个光照空间变换矩阵。
     * 这些矩阵用于将场景中的顶点从世界空间转换到光源的剪裁空间，
     * 以便渲染到立方体阴影贴图的每个面。
     * @param lightPos 光源在世界空间中的位置。
     * @param nearPlane 阴影贴图投影的近裁剪面距离。
     * @param farPlane 阴影贴图投影的远裁剪面距离。
     * @param shadowMapWidth 阴影贴图的宽度（用于计算宽高比）。
     * @param shadowMapHeight 阴影贴图的高度（用于计算宽高比）。
     * @return 包含六个光照空间变换矩阵的向量。
     */
    std::vector<Eigen::Matrix4f> CalculatePointLightSpaceMatrices(
        const Eigen::Vector3f& lightPos,
        float nearPlane,
        float farPlane,
        int shadowMapWidth,
        int shadowMapHeight);

} // namespace ShadowUtils

#endif // SHADOW_UTILS_H