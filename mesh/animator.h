#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <assimp/scene.h> // 需要 Assimp 的场景结构来解析数据
#include <assimp/matrix4x4.h> // 需要 Assimp 的矩阵类型

#include "Bone.h"         // 包含我们刚刚定义的 Bone 结构体
#include "Animation.h"    // 包含我们刚刚定义的 Animation 类
#include "AssimpUtils.h"  // 包含 Assimp 到 Eigen 的转换工具

// 定义最大骨骼数量，用于Shader中的Uniform数组大小
// 这是一个常见的限制，你需要根据你的模型和Shader能力调整
#define MAX_BONES 100 

class Animator
{
public:
    Animator();
    ~Animator();

    // 初始化Animator，从Assimp场景中解析骨骼层次和动画数据
    // scene: Assimp加载的aiScene指针
    // boneInfoMap: 一个map，存储每个骨骼的ID和它的逆绑定姿态矩阵
    //              （这个map需要在Model::processMesh中填充，并传递进来）
    void Init(const aiScene* scene, std::map<std::string, int>& boneInfoMap);

    // 更新动画状态
    // deltaTime: 帧时间差
    void UpdateAnimation(float deltaTime);

    // 播放指定名称的动画
    void PlayAnimation(const std::string& animationName);

    // 停止当前动画
    void StopAnimation();

    // 获取用于Shader的最终骨骼变换矩阵数组
    const std::vector<Eigen::Matrix4f>& GetFinalBoneMatrices() const { 
        return finalBoneMatrices_; 
    }

    // 获取当前播放的动画
    Animation* GetCurrentAnimation() const { return currentAnimation_; }

    // 获取骨骼信息映射 (名称 -> ID)
    const std::map<std::string, int>& GetBoneInfoMap() const { return boneInfoMap_; }

private:
    std::vector<Bone> bones_; // 存储所有骨骼的扁平列表
    Bone* rootBone_ = nullptr; // 骨骼层次的根节点

    std::map<std::string, int> boneInfoMap_; // 存储骨骼名称到其ID的映射

    std::map<std::string, std::unique_ptr<Animation>> animations_; // 存储所有加载的动画

    Animation* currentAnimation_ = nullptr; // 当前正在播放的动画
    float currentTime_ = 0.0f;              // 当前动画时间 (秒)
    float animationPlaySpeed_ = 1.0f;       // 动画播放速度

    // 存储最终计算出的骨骼变换矩阵，用于传递给Shader
    std::vector<Eigen::Matrix4f> finalBoneMatrices_; 

    // 私有辅助函数：
    // 递归解析Assimp节点，构建骨骼层次
    void BuildBoneHierarchy(aiNode* assimpNode, Bone* parentBone, const aiScene* scene);
    
    // 从Assimp场景中解析动画数据
    void LoadAnimations(const aiScene* scene);

    // 根据动画时间计算骨骼的局部变换（插值）
    void CalculateBoneTransform(BoneNodeAnim* boneAnim, float animationTime, Bone* bone);

    // 对Position关键帧进行插值
    Eigen::Vector3f InterpolatePosition(const BoneNodeAnim* nodeAnim, float animationTime);
    // 对Rotation关键帧进行插值
    Eigen::Quaternionf InterpolateRotation(const BoneNodeAnim* nodeAnim, float animationTime);
    // 对Scaling关键帧进行插值
    Eigen::Vector3f InterpolateScaling(const BoneNodeAnim* nodeAnim, float animationTime);
};

#endif // ANIMATOR_H