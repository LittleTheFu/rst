#ifndef ASSIMP_UTILS_H
#define ASSIMP_UTILS_H

#include <Eigen/Dense>      // For Eigen::Vector3f
#include <assimp/vector3.h> // For aiVector3D
#include <assimp/types.h>

namespace AssimpUtils {

/**
 * @brief Converts an Assimp aiVector3D to an Eigen::Vector3f.
 * @param vec The aiVector3D to convert.
 * @return The converted Eigen::Vector3f.
 */
Eigen::Vector3f ConvertAssimpVec3ToEigen(const aiVector3D& vec);

/**
 * @brief Converts an Assimp aiColor33D to an Eigen::Vector3f.
 * @param color The aiColor3D to convert.
 * @return The converted Eigen::Vector3f.
 */
Eigen::Vector3f ConvertAssimpColor3DToEigen(const aiColor3D& color);

// You can add more conversion functions here as needed
// Eigen::Matrix4f ConvertAssimpMatrix4x4ToEigen(const aiMatrix4x4& mat);

} // namespace AssimpUtils

#endif // ASSIMP_UTILS_H