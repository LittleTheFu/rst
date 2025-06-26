#include "Ray.h"
#include <limits>    // For std::numeric_limits
#include <algorithm> // For std::min/max
#include <cmath>     // For std::fabs

// 构造函数：归一化方向向量
Ray::Ray(const Eigen::Vector3f& origin, const Eigen::Vector3f& direction)
    : origin_(origin), direction_(direction.normalized()) {
    // 确保方向向量已归一化
}

// 射线与 AABB 相交检测 (Slab 方法)
bool Ray::intersectsAABB(const AABB& aabb, float& outT_min, float& outT_max) const {
    outT_min = std::numeric_limits<float>::lowest();  // 负无穷
    outT_max = std::numeric_limits<float>::max();     // 正无穷

    // 获取 AABB 的最小和最大点
    Eigen::Vector3f aabbMin = aabb.GetMin();
    Eigen::Vector3f aabbMax = aabb.GetMax();

    // 遍历 x, y, z 三个轴
    for (int i = 0; i < 3; ++i) {
        // 防止除以零，处理射线与轴平行的特殊情况
        // 使用一个小的 epsilon 来避免浮点误差导致的除以零或非常大的值
        float invDir = direction_[i];
        if (std::fabs(invDir) < std::numeric_limits<float>::epsilon()) {
            // 射线平行于当前轴的平面
            // 检查射线原点是否在 AABB 的这个轴的范围内
            if (origin_[i] < aabbMin[i] || origin_[i] > aabbMax[i]) {
                return false; // 射线平行于平面且在平面外，不相交
            }
            // 如果平行且在范围内，则此轴上的相交区间是 (-inf, inf)，不影响 tMin/tMax
            // 继续下一个轴的检查
        } else {
            invDir = 1.0f / direction_[i];
            float t0 = (aabbMin[i] - origin_[i]) * invDir;
            float t1 = (aabbMax[i] - origin_[i]) * invDir;

            // 确保 t0 总是较小的那个
            if (t0 > t1) {
                std::swap(t0, t1);
            }

            // 更新射线的交点范围
            outT_min = std::max(outT_min, t0);
            outT_max = std::min(outT_max, t1);

            // 如果范围无效，则不相交
            if (outT_min > outT_max) {
                return false;
            }
        }
    }

    // 检查交点是否在射线前方 (t_max >= 0.0f)
    // 如果交点都在射线起点的后方，我们通常不认为它“相交”
    // 注意：如果射线从AABB内部发出，outT_min可能为负，但outT_max仍可能为正，表示射线穿出AABB
    // 这里我们只关心是否存在一个正向的相交（即outT_max大于等于0），且outT_min不大于outT_max
    if (outT_max < 0.0f) {
        return false; // AABB 在射线的反方向
    }
    
    // 如果 outT_min 为负，表示射线起点在AABB内部，我们通常也认为它相交
    // 此时，outT_min 应该是进入点（即0），因为我们只关心从起点开始的距离
    // 如果 outT_min > outT_max 在上面已经返回false了

    return true; // 射线相交 AABB
}