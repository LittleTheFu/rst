#ifndef _POINT_LIGHT_DATA_FOR_UBO_H_
#define _POINT_LIGHT_DATA_FOR_UBO_H

#include <Eigen/Dense>

struct PointLightDataForUBO
{
    Eigen::Vector3f position;
    float _padding1; // 为了对齐 color 到 16 字节

    Eigen::Vector3f color;
    // float _padding2; // 为了对齐 intensity 到 16 字节

    float intensity;
    // float _padding3[3]; // 填充到 16 字节

    float constant;
    // float _padding4[3]; // 填充到 16 字节

    float linear;
    // float _padding5[3]; // 填充到 16 字节

    float quadratic;
    // float _padding6[3]; // 填充到 16 字节
};

#endif