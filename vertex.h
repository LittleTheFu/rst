#ifndef _VERTEX_H_
#define _VERTEX_H_

#include <Eigen/Dense>

class Vertex
{
public:
    Eigen::Vector3f position;
    Eigen::Vector3f normal;
    Eigen::Vector2f texCoords;
};

class ScreenVertex
{
public:
    Eigen::Vector2f position;
    Eigen::Vector2f texCoords;

    // 可以添加构造函数或其他辅助方法
};

#endif