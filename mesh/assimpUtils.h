#ifndef ASSIMP_UTILS_H
#define ASSIMP_UTILS_H

#include <Eigen/Dense>
#include <assimp/vector3.h>
#include <assimp/matrix4x4.h> // <-- 新增：包含aiMatrix4x4
#include <assimp/quaternion.h> // <-- 新增：包含aiQuaternion
#include <assimp/types.h>

namespace AssimpUtils
{

    Eigen::Vector3f ConvertAssimpVec3ToEigen(const aiVector3D &vec);

    Eigen::Vector3f ConvertAssimpColor3DToEigen(const aiColor3D &color);

    // ======== 新增用于骨骼动画的辅助函数 ========
    Eigen::Matrix4f ConvertAssimpMat4ToEigen(const aiMatrix4x4 &mat);
    Eigen::Quaternionf ConvertAssimpQuatToEigen(const aiQuaternion &quat);
    // ===========================================

}

#endif