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

#endif