#include "DebugBox.h"
#include <iostream> // For debug output, if needed
#include <debug_utils.h>
// #include "debug_utils.h" // 如果你有自己的 GL_CHECK_ERROR() 工具，可以在这里包含

DebugBox::DebugBox() {
    setup(); // 在构造函数中完成初始化
}

void DebugBox::setup() {
    // 8 个顶点定义一个单位立方体
    // 假设中心在原点 (0,0,0)，边长为 1.0，所以范围是 [-0.5, 0.5]
    float vertices[] = {
        // Positions (x, y, z)
        -0.5f, -0.5f, -0.5f, // 0: Left-Bottom-Back
         0.5f, -0.5f, -0.5f, // 1: Right-Bottom-Back
        -0.5f,  0.5f, -0.5f, // 2: Left-Top-Back
         0.5f,  0.5f, -0.5f, // 3: Right-Top-Back
        -0.5f, -0.5f,  0.5f, // 4: Left-Bottom-Front
         0.5f, -0.5f,  0.5f, // 5: Right-Bottom-Front
        -0.5f,  0.5f,  0.5f, // 6: Left-Top-Front
         0.5f,  0.5f,  0.5f  // 7: Right-Top-Front
    };

    // 12 条边的索引，每条边由两个顶点组成
    // 绘制线框需要使用 GL_LINES 模式，每两个索引构成一条线
    unsigned int indices[] = {
        // Back face
        0, 1, // Edge from (-x, -y, -z) to (+x, -y, -z)
        0, 2, // Edge from (-x, -y, -z) to (-x, +y, -z)
        0, 4, // Edge from (-x, -y, -z) to (-x, -y, +z)

        // Front face
        4, 5, // Edge from (-x, -y, +z) to (+x, -y, +z)
        4, 6, // Edge from (-x, -y, +z) to (-x, +y, +z)
        
        // Right face
        1, 3, // Edge from (+x, -y, -z) to (+x, +y, -z)
        1, 5, // Edge from (+x, -y, -z) to (+x, -y, +z)
        
        // Left face
        2, 3, // Edge from (-x, +y, -z) to (+x, +y, -z)
        2, 6, // Edge from (-x, +y, -z) to (-x, +y, +z)
        
        // Top face
        3, 7, // Edge from (+x, +y, -z) to (+x, +y, +z)
        6, 7, // Edge from (-x, +y, +z) to (+x, +y, +z)
        
        // Bottom face
        5, 7,  // Edge from (+x, -y, +z) to (+x, +y, +z) -- 这是错的，应该是 5到7是斜线。5-》 7是 5,1 5,4 7,3 7,6
        // 修正后的最后一条边 (或者说，确保所有边都绘制到)
        // 从之前的代码中补充正确的连接
        // 确保连接所有角点形成12条边
        // Correct set of indices for 12 edges:
        // Bottom face
        0, 1, // (-x,-y,-z) to (x,-y,-z)
        1, 5, // (x,-y,-z) to (x,-y,z)
        5, 4, // (x,-y,z) to (-x,-y,z)
        4, 0, // (-x,-y,z) to (-x,-y,-z)

        // Top face
        2, 3, // (-x,y,-z) to (x,y,-z)
        3, 7, // (x,y,-z) to (x,y,z)
        7, 6, // (x,y,z) to (-x,y,z)
        6, 2, // (-x,y,z) to (-x,y,-z)

        // Vertical edges
        0, 2, // (-x,-y,-z) to (-x,y,-z)
        1, 3, // (x,-y,-z) to (x,y,-z)
        4, 6, // (-x,-y,z) to (-x,y,z)
        5, 7  // (x,-y,z) to (x,y,z)
    };


    vao_ = std::make_unique<VertexArray>();
    vbo_ = std::make_unique<VertexBuffer>(vertices, sizeof(vertices), GL_STATIC_DRAW);
    ebo_ = std::make_unique<IndexBuffer>(indices, sizeof(indices) / sizeof(unsigned int), GL_STATIC_DRAW);

    vao_->bind();
    vao_->setIndexBuffer(*ebo_); // 设置索引缓冲区
    
    // 只有一个属性：位置 (layout location 0)
    // 每个顶点有 3 个浮点数 (x, y, z)，所以 stride 是 3 * sizeof(float)
    vao_->setAttribute(0, *vbo_, 3, GL_FLOAT, GL_FALSE, 0, 3 * sizeof(float), 0);

    vao_->unbind();
    // GL_CHECK_ERROR(); // 如果你有这个宏
}

void DebugBox::render() const {
    vao_->bind();
    // 使用 GL_LINES 模式和 EBO 绘制线段
    // EBO_->getCount() 返回索引的数量，这里是 24 (12条线 * 2个索引/线)
    glDrawElements(GL_LINES, ebo_->getCount(), GL_UNSIGNED_INT, 0);
    vao_->unbind();
    GL_CHECK_ERROR(); // 如果你有这个宏
}