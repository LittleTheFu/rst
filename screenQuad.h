#ifndef SCREEN_QUAD_H
#define SCREEN_QUAD_H

#include <memory> // For std::unique_ptr
#include "VertexArray.h" // 假设你有 VertexArray 和 VertexBuffer 封装
#include "VertexBuffer.h"

class ScreenQuad {
public:
    ScreenQuad(); // 构造函数，负责初始化 VAO/VBO
    ~ScreenQuad() = default; // 智能指针会自动管理资源，所以默认析构即可

    // 绘制四边形的方法
    void render() const;

private:
    std::unique_ptr<VertexArray> vao_;
    std::unique_ptr<VertexBuffer> vbo_;

    // 私有辅助方法来设置 OpenGL 缓冲区
    void setup();
};

#endif // SCREEN_QUAD_H