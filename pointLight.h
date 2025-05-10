#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include <Eigen/Dense>

struct PointLight
{
    Eigen::Vector3f position;
    Eigen::Vector3f color;
    float intensity = 1.0f;     // 光照强度
    float constant = 1.0f;      // 衰减常数项
    float linear = 0.09f;       // 衰减线性项
    float quadratic = 0.032f;    // 衰减二次项
};

#endif // POINTLIGHT_H