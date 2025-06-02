#include "ScreenQuad.h"
#include <glad/glad.h> // 或者你使用的其他 OpenGL 加载库
#include "debug_utils.h" // 用于 GL_CHECK_ERROR()

ScreenQuad::ScreenQuad() {
    setup(); // 在构造函数中完成初始化
}

void ScreenQuad::setup() {
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f, 1.0f, 0.0f, 1.0f, // Top-left
        -1.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
        1.0f, -1.0f, 1.0f, 0.0f,  // Bottom-right

        -1.0f, 1.0f, 0.0f, 1.0f,  // Top-left (重复)
        1.0f, -1.0f, 1.0f, 0.0f,  // Bottom-right (重复)
        1.0f, 1.0f, 1.0f, 1.0f    // Top-right
    };

    vao_ = std::make_unique<VertexArray>();
    vbo_ = std::make_unique<VertexBuffer>(quadVertices, sizeof(quadVertices), GL_STATIC_DRAW);

    vao_->bind();
    // 属性设置
    // 位置属性 (layout location 0)
    vao_->setAttribute(0, *vbo_, 2, GL_FLOAT, GL_FALSE, 0, 4 * sizeof(float), 0);
    // 纹理坐标属性 (layout location 1)
    vao_->setAttribute(1, *vbo_, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 4 * sizeof(float), 0);

    vao_->unbind();
    GL_CHECK_ERROR();
}

void ScreenQuad::render() const {
    vao_->bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    vao_->unbind();
    GL_CHECK_ERROR();
}