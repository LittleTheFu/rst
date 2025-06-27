#ifndef ASSIMP_UTILS_H
#define ASSIMP_UTILS_H

#include <Eigen/Dense>
#include <assimp/vector3.h>
#include <assimp/types.h>

namespace AssimpUtils
{

    Eigen::Vector3f ConvertAssimpVec3ToEigen(const aiVector3D &vec);

    Eigen::Vector3f ConvertAssimpColor3DToEigen(const aiColor3D &color);

}

#endif