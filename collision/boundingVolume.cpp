#include "BoundingVolume.h"
#include <limits>      // For std::numeric_limits
#include <algorithm>   // For std::min/max
#include "Ray.h"       // <-- 新增：包含 Ray 的定义，以便使用 Ray 对象

// AABB 射线相交测试的实现
// 现在 AABB 接受 Ray 对象，并调用 Ray 的相交方法
bool AABB::IntersectsRay(const Ray& ray, float& outT) const {
    float t_min_val, t_max_val; // Ray::intersectsAABB 需要这两个参数
    bool result = ray.intersectsAABB(*this, t_min_val, t_max_val);

    if (result) {
        // 如果相交，我们通常只关心进入 AABB 的最近距离（正向）
        // 如果射线起点在 AABB 内部，t_min_val 可能为负，此时我们应返回 0
        outT = (t_min_val < 0.0f) ? 0.0f : t_min_val;
    }
    return result;
}

// AABB 变换的实现 (保持不变)
AABB* AABB::Transform(const Eigen::Matrix4f& matrix) const {
    Eigen::Vector3f corners[8];
    corners[0] = min_;
    corners[1] = Eigen::Vector3f(max_.x(), min_.y(), min_.z());
    corners[2] = Eigen::Vector3f(min_.x(), max_.y(), min_.z());
    corners[3] = Eigen::Vector3f(min_.x(), min_.y(), max_.z());
    corners[4] = Eigen::Vector3f(max_.x(), max_.y(), min_.z());
    corners[5] = Eigen::Vector3f(max_.x(), min_.y(), max_.z());
    corners[6] = Eigen::Vector3f(min_.x(), max_.y(), max_.z());
    corners[7] = max_;

    Eigen::Vector3f newMin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Eigen::Vector3f newMax(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());

    for (int i = 0; i < 8; ++i) {
        Eigen::Vector4f transformedCorner = matrix * Eigen::Vector4f(corners[i].x(), corners[i].y(), corners[i].z(), 1.0f);
        Eigen::Vector3f actualTransformedCorner = transformedCorner.head<3>() / transformedCorner.w();

        newMin = newMin.cwiseMin(actualTransformedCorner);
        newMax = newMax.cwiseMax(actualTransformedCorner);
    }
    
    return new AABB(newMin, newMax);
}