#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include <Eigen/Dense>

struct PointLight
{
    Eigen::Vector3f position;
    Eigen::Vector3f color;
    float intensity = 1.0f;
};

#endif