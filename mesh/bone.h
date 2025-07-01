#ifndef BONE_H
#define BONE_H

#include <string>
#include <vector>
#include <map> // 用于存储关键帧索引
#include <Eigen/Dense>
#include <Eigen/Geometry>

// 前向声明，因为 BoneNodeAnim 会在 Animation 中被引用
struct BoneNodeAnim;

// Bone 结构体，代表骨架中的一个关节
struct Bone
{
    std::string name; // 骨骼名称，与 Assimp 的节点名称对应
    int id;           // 骨骼的唯一 ID，通常用于索引骨骼矩阵数组

    // 绑定姿态的逆矩阵：将顶点从模型空间变换到骨骼局部空间。
    // 这是蒙皮计算的核心。从 aiBone::mOffsetMatrix 获取。
    Eigen::Matrix4f inverseBindPoseTransform;

    // 骨骼的局部变换矩阵：它相对于父骨骼的当前变换。
    // 在动画播放时，这个矩阵会根据关键帧数据更新。
    Eigen::Matrix4f localTransform;

    // 骨骼在模型空间（世界空间，但相对于模型根节点）的全局变换矩阵。
    // 由其父骨骼的 globalTransform 乘以自身的 localTransform 计算得来。
    Eigen::Matrix4f globalTransform;

    // 指向父骨骼的指针，用于遍历骨骼层次。
    // 注意：实际实现中，通常使用索引而不是原始指针来避免悬空指针问题，
    //       但这里为了概念清晰先用指针。
    //       更健壮的方式是存储父骨骼的id或索引。
    Bone *parent = nullptr;
    std::vector<Bone *> children; // 指向子骨骼的指针列表

    Bone(const std::string &name, int id, const Eigen::Matrix4f &inverseBindPose)
        : name(name), id(id), inverseBindPoseTransform(inverseBindPose),
          localTransform(Eigen::Matrix4f::Identity()),
          globalTransform(Eigen::Matrix4f::Identity())
    {
    }

    Bone() : name(""),                                              // 空字符串
             id(-1),                                                // 无效ID
             inverseBindPoseTransform(Eigen::Matrix4f::Identity()), // 单位矩阵
             localTransform(Eigen::Matrix4f::Identity()),           // 单位矩阵
             globalTransform(Eigen::Matrix4f::Identity())           // 单位矩阵
    {
        // parent 已经在成员声明中默认初始化为 nullptr
    }

    // 递归地更新骨骼的全局变换矩阵
    void UpdateGlobalTransform()
    {
        if (parent)
        {
            // 如果有父骨骼，全局变换 = 父骨骼的全局变换 * 自身局部变换
            globalTransform = parent->globalTransform * localTransform;
        }
        else
        {
            // 如果是根骨骼，全局变换 = 自身局部变换
            globalTransform = localTransform;
        }

        // 递归更新子骨骼
        for (Bone *child : children)
        {
            child->UpdateGlobalTransform();
        }
    }
};

#endif // BONE_H