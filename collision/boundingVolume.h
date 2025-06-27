#ifndef AABB_H // 将宏名称从 BOUNDING_VOLUME_H 更改为 AABB_H
#define AABB_H

#include <Eigen/Dense>      // 包含 Eigen 库
#include <limits>           // For std::numeric_limits
#include <algorithm>        // For std::min/max
#include <memory>           // For std::unique_ptr

// 前向声明 Ray 类
class Ray; 

/**
 * @brief 轴对齐包围盒 (AABB) 类。
 * 不再继承自 BoundingVolume。
 */
class AABB {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW 

    // 默认构造函数，初始化为无效状态（可被任何点扩展）
    AABB()
        : min_(Eigen::Vector3f::Constant(std::numeric_limits<float>::max())),
          max_(Eigen::Vector3f::Constant(std::numeric_limits<float>::lowest())) {}

    // 构造函数，使用给定的最小和最大点初始化
    AABB(const Eigen::Vector3f& min, const Eigen::Vector3f& max) : min_(min), max_(max) {}

    // 拷贝构造函数
    AABB(const AABB& other) : min_(other.min_), max_(other.max_) {}

    // ~AABB() = default; // 默认析构函数，如果不需要特殊清理可以不显式声明

    // 获取 AABB 的最小点
    Eigen::Vector3f GetMin() const { return min_; }
    // 获取 AABB 的最大点
    Eigen::Vector3f GetMax() const { return max_; }

    // 设置 AABB 的最小点
    void SetMin(const Eigen::Vector3f& min) { min_ = min; }
    // 设置 AABB 的最大点
    void SetMax(const Eigen::Vector3f& max) { max_ = max; }

    // 获取 AABB 的尺寸 (max - min)
    Eigen::Vector3f GetSize() const { return max_ - min_; }

    // 获取 AABB 的中心点
    Eigen::Vector3f GetCenter() const { return (min_ + max_) / 2.0f; }

    /**
     * @brief 扩展 AABB 以包含给定的点。
     * @param point 要包含的点。
     */
    void Extend(const Eigen::Vector3f& point) {
        min_ = min_.cwiseMin(point);
        max_ = max_.cwiseMax(point);
    }

    /**
     * @brief 扩展 AABB 以包含另一个 AABB。
     * @param other 另一个 AABB。
     */
    void Extend(const AABB& other) {
        Extend(other.min_);
        Extend(other.max_);
    }

    /**
     * @brief 判断射线是否与 AABB 相交。
     * @param ray 要检测的 Ray 对象。
     * @param outT 如果相交，返回交点距离射线起点的参数值（沿着射线方向）。
     * @return 如果射线与 AABB 相交，返回 true；否则返回 false。
     */
    // 移除 override 关键字
    bool IntersectsRay(const Ray& ray, float& outT) const;

    /**
     * @brief 将 AABB 通过给定的变换矩阵进行变换。
     * 返回一个新的 AABB，表示变换后的形状，并由 unique_ptr 管理所有权。
     * @param matrix 用于变换的 4x4 变换矩阵。
     * @return 变换后的 AABB 对象，由 unique_ptr 封装。
     */
    // 移除 override 关键字
    std::unique_ptr<AABB> Transform(const Eigen::Matrix4f& matrix) const; 

public: 
    Eigen::Vector3f min_;
    Eigen::Vector3f max_;
};

#endif // AABB_H