#ifndef ANIMATION_H
#define ANIMATION_H

#include <string>
#include <vector>
#include <map>
#include <Eigen/Dense>
#include <Eigen/Geometry> // For Eigen::Quaternionf

// 结构体，表示一个时间戳下的位置关键帧
struct PositionKey {
    Eigen::Vector3f position;
    double time; // 动画时间 (ticks)
};

// 结构体，表示一个时间戳下的旋转关键帧
struct RotationKey {
    Eigen::Quaternionf rotation;
    double time; // 动画时间 (ticks)
};

// 结构体，表示一个时间戳下的缩放关键帧
struct ScalingKey {
    Eigen::Vector3f scale;
    double time; // 动画时间 (ticks)
};

// 结构体，代表单个骨骼的动画通道
// 对应 Assimp 的 aiNodeAnim
struct BoneNodeAnim {
    std::string boneName; // 骨骼名称
    std::vector<PositionKey> positionKeys; // 位置关键帧列表
    std::vector<RotationKey> rotationKeys; // 旋转关键帧列表
    std::vector<ScalingKey> scalingKeys;   // 缩放关键帧列表

    // 辅助函数，用于查找给定时间点左右两侧的关键帧索引
    size_t GetPositionIndex(double animationTime) const {
        for (size_t i = 0; i < positionKeys.size() - 1; ++i) {
            if (animationTime < positionKeys[i+1].time) {
                return i;
            }
        }
        return 0; // 如果时间超出范围，返回第一个
    }

    size_t GetRotationIndex(double animationTime) const {
        for (size_t i = 0; i < rotationKeys.size() - 1; ++i) {
            if (animationTime < rotationKeys[i+1].time) {
                return i;
            }
        }
        return 0;
    }

    size_t GetScalingIndex(double animationTime) const {
        for (size_t i = 0; i < scalingKeys.size() - 1; ++i) {
            if (animationTime < scalingKeys[i+1].time) {
                return i;
            }
        }
        return 0;
    }
};

// Animation 类，代表一个完整的动画片段
class Animation
{
public:
    std::string name; // 动画名称
    double duration;  // 动画总时长 (ticks)
    double ticksPerSecond; // 每秒的 ticks 数

    // 存储每个骨骼的动画通道数据，用骨骼名称映射
    std::map<std::string, BoneNodeAnim> boneAnimations; 

    Animation(const std::string& name, double duration, double ticksPerSecond)
        : name(name), duration(duration), ticksPerSecond(ticksPerSecond)
    {}

    // 根据动画名称获取 BoneNodeAnim
    BoneNodeAnim* FindBoneNodeAnim(const std::string& boneName) {
        auto it = boneAnimations.find(boneName);
        if (it != boneAnimations.end()) {
            return &(it->second);
        }
        return nullptr;
    }
};

#endif // ANIMATION_H