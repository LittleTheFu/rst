#include "utilities.h"

Eigen::Vector3f Utilities::worldToScreenSpace(const Eigen::Vector3f &worldPos,
                                              const Eigen::Matrix4f &viewMatrix,
                                              const Eigen::Matrix4f &projMatrix)
{
    Eigen::Vector4f worldPosHomo(worldPos.x(), worldPos.y(), worldPos.z(), 1.0f);

    // MVP 变换：世界空间 -> 裁剪空间
    Eigen::Vector4f clipSpace = projMatrix * viewMatrix * worldPosHomo;

    // 避免除以零
    if (clipSpace.w() == 0.0f)
        return Eigen::Vector3f(0.0f, 0.0f, 1.0f); // 或 return invalid 值

    // 裁剪空间 -> NDC [-1, 1]
    Eigen::Vector3f ndc = clipSpace.head<3>() / clipSpace.w();

    // NDC -> 屏幕空间 [0, 1]
    float screenX = ndc.x() * 0.5f + 0.5f;
    float screenY = ndc.y() * 0.5f + 0.5f;
    float screenZ = ndc.z(); // NDC 深度

    return Eigen::Vector3f(screenX, screenY, screenZ);
}