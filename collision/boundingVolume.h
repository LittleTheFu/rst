#ifndef BOUNDING_VOLUME_H
#define BOUNDING_VOLUME_H

#include <Eigen/Dense>      // 包含 Eigen 库
#include <limits>           // For std::numeric_limits
#include <algorithm>        // For std::min/max

// 前向声明 Ray 类
// 注意：如果 Ray 类依赖 BoundingVolume（通过 AABB），这里可能需要调整包含顺序或使用前向声明
// 为了 IntersectsRay(const Ray&) 签名，需要先声明 Ray
class Ray; // <-- 新增前向声明

/**
 * @brief 抽象的包围体基类。
 * 定义了所有包围体应具备的通用接口。
 */
class BoundingVolume {
public:
    // 虚析构函数，确保派生类正确析构
    virtual ~BoundingVolume() = default;

    /**
     * @brief 判断射线是否与包围体相交。
     * @param ray 要检测的 Ray 对象。
     * @param outT 如果相交，返回交点距离射线起点的参数值（沿着射线方向）。
     * @return 如果射线与包围体相交，返回 true；否则返回 false。
     */
    virtual bool IntersectsRay(const Ray& ray, float& outT) const = 0; // <-- 修改了参数

    /**
     * @brief 将包围体通过给定的变换矩阵进行变换。
     * 返回一个新的包围体，表示变换后的形状。
     * @param matrix 用于变换的 4x4 变换矩阵。
     * @return 变换后的 BoundingVolume 对象。具体类型由派生类决定。
     */
    virtual BoundingVolume* Transform(const Eigen::Matrix4f& matrix) const = 0; // 返回指针，需要delete

    /**
     * @brief 调试可视化功能（可选）。
     * 通常用于在调试模式下绘制包围体，方便可视化检查。
     */
    virtual void DebugDraw() const {
        // 默认实现为空，派生类可覆盖
    }
};


/**
 * @brief 轴对齐包围盒 (AABB) 类。
 * 继承自 BoundingVolume，实现了 AABB 特有的功能。
 */
class AABB : public BoundingVolume {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW 

    // 默认构造函数，初始化为无效状态（可被任何点扩展）
    AABB()
        : min_(Eigen::Vector3f::Constant(std::numeric_limits<float>::max())),
          max_(Eigen::Vector3f::Constant(std::numeric_limits<float>::lowest())) {}

    // 构造函数，使用给定的最小和最大点初始化
    AABB(const Eigen::Vector3f& min, const Eigen::Vector3f& max) : min_(min), max_(max) {}

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

    // 实现 BoundingVolume 接口的虚函数
    // 注意：这里需要包含 Ray.h，或者在 AABB.cpp 中包含。
    // 为了让编译器知道 Ray 的完整定义，通常在 .cpp 文件中包含。
    bool IntersectsRay(const Ray& ray, float& outT) const override; // <-- 修改了参数
    AABB* Transform(const Eigen::Matrix4f& matrix) const override; 

private:
    Eigen::Vector3f min_;
    Eigen::Vector3f max_;
};

#endif // BOUNDING_VOLUME_H