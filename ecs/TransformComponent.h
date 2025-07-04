#ifndef TRANSFORM_COMPONENT_H
#define TRANSFORM_COMPONENT_H

#include <Eigen/Dense> // 包含 Eigen 库，用于向量和矩阵操作

/**
 * @brief TransformComponent 存储一个实体的变换信息。
 * 包含位置、旋转（欧拉角）和缩放。
 */
struct TransformComponent {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW // 如果使用 Eigen 类型作为结构体成员，推荐添加此宏

    Eigen::Vector3f position;  // 世界空间位置
    Eigen::Vector3f rotation;  // 欧拉角 (Roll, Pitch, Yaw) 或 (X, Y, Z 轴旋转，弧度或度数取决于你的习惯)
                               // 在 RenderSystem 中，我假设它们是弧度并转换为四元数
    Eigen::Vector3f scale;     // 缩放因子 (X, Y, Z)

    /**
     * @brief 默认构造函数，初始化为单位变换。
     */
    TransformComponent()
        : position(Eigen::Vector3f::Zero()),
          rotation(Eigen::Vector3f::Zero()),
          scale(Eigen::Vector3f::Ones()) {} // 默认缩放为 (1, 1, 1)

    /**
     * @brief 构造函数，使用指定的位置、旋转和缩放初始化。
     * @param pos 初始位置。
     * @param rot 初始旋转 (欧拉角)。
     * @param s 初始缩放。
     */
    TransformComponent(const Eigen::Vector3f& pos, const Eigen::Vector3f& rot, const Eigen::Vector3f& s)
        : position(pos),
          rotation(rot),
          scale(s) {}

    // 你可以根据需要添加辅助方法，例如：
    // Eigen::Matrix4f getTransformMatrix() const; // 返回基于这些属性的变换矩阵
};

#endif // TRANSFORM_COMPONENT_H