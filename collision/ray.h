#ifndef _RAY_H_
#define _RAY_H_

#include <Eigen/Dense> // For Eigen::Vector3f
#include "BoundingVolume.h" // 包含 AABB 的定义

/**
 * @brief 表示一条射线，用于光线投射和相交检测。
 * 射线由起点和归一化方向向量定义。
 */
class Ray {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW // 如果 Ray 内部使用了Eigen固定大小的数据结构，建议添加此宏以保证内存对齐

    /**
     * @brief 构造函数。
     * @param origin 射线的起点。
     * @param direction 射线的方向（会被内部归一化）。
     */
    Ray(const Eigen::Vector3f& origin, const Eigen::Vector3f& direction);

    /**
     * @brief 获取射线的起点。
     * @return 射线的起点向量。
     */
    Eigen::Vector3f getOrigin() const { return origin_; }

    /**
     * @brief 获取射线的归一化方向。
     * @return 射线的归一化方向向量。
     */
    Eigen::Vector3f getDirection() const { return direction_; }

    /**
     * @brief 检测射线是否与轴对齐包围盒 (AABB) 相交。
     * 使用优化的 Slab 方法。
     * @param aabb 要检测的 AABB 对象。
     * @param outT_min 如果相交，返回射线进入 AABB 的最小参数值。
     * @param outT_max 如果相交，返回射线离开 AABB 的最大参数值。
     * @return 如果射线与 AABB 相交，返回 true；否则返回 false。
     */
    bool intersectsAABB(const AABB& aabb, float& outT_min, float& outT_max) const;

    // TODO: 根据需要添加其他相交检测方法，例如：
    // bool intersectsTriangle(const Eigen::Vector3f& v0, const Eigen::Vector3f& v1, const Eigen::Vector3f& v2, float& t) const;
    // bool intersectsSphere(const Eigen::Vector3f& center, float radius, float& t) const;

private:
    Eigen::Vector3f origin_;    // 射线的起点
    Eigen::Vector3f direction_; // 射线的归一化方向 (已归一化)
};

#endif // RAY_H