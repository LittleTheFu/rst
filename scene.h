#ifndef _SCENE_H_
#define _SCENE_H_

#include "array"
#include "vertex.h"
#include <string>

class Scene
{
public:
    void init();
    void run();

private:
    std::array<Vertex, 3> vertices_;

    std::string vertex_shader_;
    std::string fragment_shader_;

    unsigned int shaderProgram_;
    unsigned int VAO_;
    unsigned int VBO_;
};

#endif