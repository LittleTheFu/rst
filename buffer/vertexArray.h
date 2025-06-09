#ifndef VERTEX_ARRAY_H
#define VERTEX_ARRAY_H

#include "GLResource.h"
#include "VertexBuffer.h" // 需要包含 VertexBuffer 来传递其对象
#include "IndexBuffer.h"  // 需要包含 IndexBuffer 来传递其对象
#include <glad/glad.h>
#include <cstddef> // For size_t
#include <vector>   // For internal tracking if needed, though often not strictly necessary for simple VAO

class VertexArray : public GLResource {
public:
    VertexArray(); // 构造函数

    // 禁用复制构造函数和赋值运算符，防止资源共享导致的 double free
    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;

    // 移动构造函数和移动赋值运算符
    VertexArray(VertexArray&& other) noexcept;
    VertexArray& operator=(VertexArray&& other) noexcept;

    // --- 核心功能：设置顶点属性 ---
    // bind() 和 unbind() 不再是 override，因为 GLResource 不包含它们
    void bind() const;
    void unbind() const;

    // --- 核心功能：设置顶点属性 ---
    // attribIndex: 顶点属性的location (shader中的layout(location=X))
    // vbo: 对应的 VertexBuffer 对象
    // size: 属性的分量数量 (如vec3是3, vec2是2)
    // type: 属性的数据类型 (如GL_FLOAT, GL_UNSIGNED_INT)
    // normalized: 是否需要归一化 (GL_TRUE/GL_FALSE)，通常用于整数类型
    // relativeOffset: 属性在单个顶点结构中的偏移量 (字节)
    // stride: 整个单个顶点的总字节大小 (如果VBO是交错存储的，通常是sizeof(VertexStruct))
    // bindingIndex: VBO的绑定点索引 (VAO中可以有多个绑定点，通常从0开始递增)
   void setAttribute(
        GLuint attribIndex,
        const VertexBuffer& vbo,
        GLint size,
        GLenum type,
        GLboolean normalized,
        GLuint relativeOffset,
        GLsizei stride,           // <-- 添加这个参数
        GLuint bindingIndex
    );

    // 设置属性的步长（如果VBO是交错存储的，这通常在setAttribute中完成）
    // 或者独立设置 VBO 绑定点属性
    void setBindingDivisor(GLuint bindingIndex, GLuint divisor);


    // 启用/禁用顶点属性
    void enableAttribute(GLuint attribIndex);
    void disableAttribute(GLuint attribIndex);

    // 绑定 IndexBuffer 到此 VAO
    void setIndexBuffer(const IndexBuffer& ibo);

protected:
    void release() override;
};

#endif // VERTEX_ARRAY_H