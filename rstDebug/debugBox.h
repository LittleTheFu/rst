#ifndef DEBUG_BOX_H
#define DEBUG_BOX_H

#include <memory>         // For std::unique_ptr
#include <glad/glad.h>    // For OpenGL types like GL_STATIC_DRAW, GL_UNSIGNED_INT etc.
#include "VertexArray.h"  // 假设你有 VertexArray 和 VertexBuffer 封装
#include "VertexBuffer.h"
#include "IndexBuffer.h"  // 需要 IndexBuffer 来绘制线框立方体

/**
 * @brief 用于调试渲染的单位立方体线框模型。
 * 封装了单位立方体的 VAO、VBO 和 EBO，以及其渲染方法。
 * 该立方体的范围通常为 [-0.5, 0.5] 在三个轴上。
 */
class DebugBox {
public:
    /**
     * @brief 构造函数，负责初始化 OpenGL 缓冲区和顶点数据。
     */
    DebugBox();

    /**
     * @brief 析构函数，智能指针会自动管理资源。
     */
    ~DebugBox() = default;

    /**
     * @brief 渲染单位立方体的线框。
     * 调用此方法前，应先绑定着色器，并设置好 MVP 矩阵和颜色等 uniform。
     */
    void render() const;

private:
    std::unique_ptr<VertexArray> vao_;
    std::unique_ptr<VertexBuffer> vbo_;
    std::unique_ptr<IndexBuffer> ebo_; // 使用 EBO 来绘制线框

    /**
     * @brief 私有辅助方法，用于设置 OpenGL 缓冲区和顶点属性。
     */
    void setup();
};

#endif // DEBUG_BOX_H