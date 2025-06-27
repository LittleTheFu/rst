#include "assimpUtils.h"

namespace AssimpUtils {

Eigen::Vector3f ConvertAssimpVec3ToEigen(const aiVector3D& vec) {
    return Eigen::Vector3f(vec.x, vec.y, vec.z);
}

Eigen::Vector3f ConvertAssimpColor3DToEigen(const aiColor3D& color) {
    return Eigen::Vector3f(color.r, color.g, color.b);
}

} // namespace AssimpUtils