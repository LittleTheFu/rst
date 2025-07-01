#ifndef _VERTEX_H_
#define _VERTEX_H_

#include <Eigen/Dense>

class Vertex
{
public:
    Eigen::Vector3f position;
    Eigen::Vector3f normal;
    Eigen::Vector2f texCoords;
    Eigen::Vector3f tangent;
    Eigen::Vector3f bitangent;
    
    // ======== 新增用于骨骼动画的成员 ========
    Eigen::Vector4i boneIDs;    // 存储受影响的骨骼ID (最多4个)
    Eigen::Vector4f boneWeights; // 存储对应骨骼的权重 (最多4个)
    // =======================================

    // 默认构造函数，确保新添加的成员有合理默认值
    Vertex() 
        : position(Eigen::Vector3f::Zero()), 
          normal(Eigen::Vector3f::Zero()), 
          texCoords(Eigen::Vector2f::Zero()), 
          tangent(Eigen::Vector3f::Zero()), 
          bitangent(Eigen::Vector3f::Zero()),
          boneIDs(Eigen::Vector4i::Zero()),    // 初始化为0
          boneWeights(Eigen::Vector4f::Zero()) // 初始化为0.0f
    {}
};

class ScreenVertex
{
public:
    Eigen::Vector2f position;
    Eigen::Vector2f texCoords;

    // 可以添加构造函数或其他辅助方法
};

#endif