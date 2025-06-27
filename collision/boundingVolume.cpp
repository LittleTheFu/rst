// 将包含的头文件从 "BoundingVolume.h" 更改为 "AABB.h"
#include "boundingVolume.h" 
#include <limits>           // For std::numeric_limits
#include <algorithm>        // For std::min/max
#include "Ray.h"            // 包含 Ray 的定义

// AABB 射线相交测试的实现
// 移除 override 关键字
bool AABB::IntersectsRay(const Ray& ray, float& outT) const {
    float t_min_val, t_max_val; 
    bool result = ray.intersectsAABB(*this, t_min_val, t_max_val);

    if (result) {
        outT = (t_min_val < 0.0f) ? 0.0f : t_min_val;
    }
    return result;
}

// AABB 变换的实现
// 移除 override 关键字
std::unique_ptr<AABB> AABB::Transform(const Eigen::Matrix4f& matrix) const {
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
        Eigen::Vector4f transformedCorner4 = matrix * Eigen::Vector4f(corners[i].x(), corners[i].y(), corners[i].z(), 1.0f);
        Eigen::Vector3f actualTransformedCorner = transformedCorner4.head<3>() / transformedCorner4.w();

        newMin = newMin.cwiseMin(actualTransformedCorner);
        newMax = newMax.cwiseMax(actualTransformedCorner);
    }
    
    return std::make_unique<AABB>(newMin, newMax);
}