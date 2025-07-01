#include "AssimpUtils.h"

namespace AssimpUtils {

Eigen::Vector3f ConvertAssimpVec3ToEigen(const aiVector3D& vec) {
    return Eigen::Vector3f(vec.x, vec.y, vec.z);
}

Eigen::Vector3f ConvertAssimpColor3DToEigen(const aiColor3D& color) {
    return Eigen::Vector3f(color.r, color.g, color.b);
}

// ======== 新增用于骨骼动画的辅助函数实现 ========
Eigen::Matrix4f ConvertAssimpMat4ToEigen(const aiMatrix4x4& mat) {
    Eigen::Matrix4f eigenMat;
    // Assimp's matrix is row-major, Eigen's default is column-major.
    // So we can directly assign elements, Assimp[row][column] to Eigen(row, column)
    // or use Eigen's row-major constructor if needed.
    // Here, directly mapping:
    eigenMat << mat.a1, mat.a2, mat.a3, mat.a4,
                mat.b1, mat.b2, mat.b3, mat.b4,
                mat.c1, mat.c2, mat.c3, mat.c4,
                mat.d1, mat.d2, mat.d3, mat.d4;
    return eigenMat;
}

Eigen::Quaternionf ConvertAssimpQuatToEigen(const aiQuaternion& quat) {
    // Assimp: (x, y, z, w), Eigen: (w, x, y, z) in constructor.
    // So, Eigen::Quaternionf(w, x, y, z)
    return Eigen::Quaternionf(quat.w, quat.x, quat.y, quat.z);
}
// =================================================
} // namespace AssimpUtils