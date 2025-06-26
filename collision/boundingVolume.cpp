#include "BoundingVolume.h"
#include <limits> // For std::numeric_limits
#include <algorithm> // For std::min/max

// AABB 射线相交测试的实现
// 使用优化的 slab method
bool AABB::IntersectsRay(const Eigen::Vector3f& rayOrigin, const Eigen::Vector3f& rayDirection, float& outT) const {
    float tMin = std::numeric_limits<float>::lowest();  // 负无穷
    float tMax = std::numeric_limits<float>::max();     // 正无穷

    // 遍历 x, y, z 三个轴
    for (int i = 0; i < 3; ++i) {
        // 计算当前轴的 slab 的两个交点参数
        float invDir = 1.0f / rayDirection[i];
        float t0 = (min_[i] - rayOrigin[i]) * invDir;
        float t1 = (max_[i] - rayOrigin[i]) * invDir;

        // 确保 t0 是较小的那个
        if (invDir < 0.0f) {
            std::swap(t0, t1);
        }

        // 更新射线的交点范围
        tMin = std::max(tMin, t0);
        tMax = std::min(tMax, t1);

        // 如果范围无效，则不相交
        if (tMin > tMax) {
            return false;
        }
    }

    // 检查交点是否在射线前方 (tMin > 0.0f)
    // 如果交点在射线起点的后方，我们通常不认为它“相交”
    // 如果需要允许射线从AABB内部射出，可以移除此检查或调整
    if (tMax < 0.0f) { // 如果最近的交点都小于0，说明AABB在射线身后
        return false;
    }

    outT = tMin; // 返回最近的交点距离
    return true;
}


// AABB 变换的实现
// 将 AABB 的 8 个角点用矩阵变换，然后重新计算新的 AABB
AABB* AABB::Transform(const Eigen::Matrix4f& matrix) const {
    // 获取当前 AABB 的 8 个角点
    Eigen::Vector3f corners[8];
    corners[0] = min_;
    corners[1] = Eigen::Vector3f(max_.x(), min_.y(), min_.z());
    corners[2] = Eigen::Vector3f(min_.x(), max_.y(), min_.z());
    corners[3] = Eigen::Vector3f(min_.x(), min_.y(), max_.z());
    corners[4] = Eigen::Vector3f(max_.x(), max_.y(), min_.z());
    corners[5] = Eigen::Vector3f(max_.x(), min_.y(), max_.z());
    corners[6] = Eigen::Vector3f(min_.x(), max_.y(), max_.z());
    corners[7] = max_;

    // 初始化新的 AABB 为无限大或无限小，以便可以被任何点扩展
    Eigen::Vector3f newMin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Eigen::Vector3f newMax(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());

    // 变换每个角点并更新新的 AABB
    for (int i = 0; i < 8; ++i) {
        Eigen::Vector4f transformedCorner = matrix * Eigen::Vector4f(corners[i].x(), corners[i].y(), corners[i].z(), 1.0f);
        
        // 投影到 3D 空间（如果 W 不为 1，需要除以 W）
        Eigen::Vector3f actualTransformedCorner = transformedCorner.head<3>() / transformedCorner.w();

        newMin = newMin.cwiseMin(actualTransformedCorner);
        newMax = newMax.cwiseMax(actualTransformedCorner);
    }
    
    // 创建并返回新的 AABB 对象
    return new AABB(newMin, newMax);
}