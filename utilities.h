#ifndef UTILITIES_H
#define UTILITIES_H

#include <Eigen/Dense>

class Utilities
{
public:
    static Eigen::Vector3f worldToScreenSpace(const Eigen::Vector3f &worldPos,
                                              const Eigen::Matrix4f &viewMatrix,
                                              const Eigen::Matrix4f &projMatrix);
};

#endif // UTILITIES_H