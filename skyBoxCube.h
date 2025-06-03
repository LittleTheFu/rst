#ifndef SKYBOX_CUBE_H
#define SKYBOX_CUBE_H

#include <memory>
#include "VertexArray.h"
#include "VertexBuffer.h"

class SkyboxCube {
public:
    SkyboxCube(); // 构造函数初始化 VAO/VBO
    ~SkyboxCube() = default;

    void render() const;

private:
    std::unique_ptr<VertexArray> vao_;
    std::unique_ptr<VertexBuffer> vbo_;

    void setup();
};

#endif // SKYBOX_CUBE_H
