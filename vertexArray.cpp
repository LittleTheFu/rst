#include "VertexArray.h"
#include <iostream> // For error messages

// 构造函数
VertexArray::VertexArray() : GLResource()
{
    glCreateVertexArrays(1, &id_); // DSA: 直接创建 VAO ID
    if (id_ == 0)
    {
        std::cerr << "Error: Failed to create VertexArray." << std::endl;
    }
}

// 移动构造函数
VertexArray::VertexArray(VertexArray &&other) noexcept
    : GLResource(std::move(other)) // 调用基类的移动构造函数
{
    // 如果基类没有移动语义，则手动转移 id_
    // id_ = other.id_;
    // other.id_ = 0; // 基类移动构造函数已经做了这个
}

// 移动赋值运算符
VertexArray &VertexArray::operator=(VertexArray &&other) noexcept
{
    if (this != &other)
    {
        release(); // 释放当前对象的资源
        // 调用基类的移动赋值运算符
        GLResource::operator=(std::move(other));
        // 如果基类没有移动语义，则手动转移 id_
        // id_ = other.id_;
        // other.id_ = 0; // 基类移动赋值运算符已经做了这个
    }
    return *this;
}

// 绑定 VAO (用于绘制前激活)
void VertexArray::bind() const
{
    glBindVertexArray(id_);
}

// 解绑 VAO
void VertexArray::unbind() const
{
    glBindVertexArray(0);
}

// 设置顶点属性 (DSA 方式)
void VertexArray::setAttribute(
    GLuint attribIndex,
    const VertexBuffer &vbo,
    GLint size,
    GLenum type,
    GLboolean normalized,
    GLuint relativeOffset,
    GLsizei stride, // <-- 添加这个参数
    GLuint bindingIndex)
{
    // 1. 将 VertexBuffer 绑定到 VAO 的一个绑定点
    // vaoID, bindingIndex, bufferID, offset (VBO内的起始偏移), stride (单个完整顶点的字节大小)
    glVertexArrayVertexBuffer(id_, bindingIndex, vbo.id(), 0, stride);

    // 2. 设置属性的格式
    // vaoID, attribIndex, size, type, normalized, relativeOffset (属性在单个顶点内的偏移)
    glVertexArrayAttribFormat(id_, attribIndex, size, type, normalized, relativeOffset);

    // 3. 将属性索引与绑定点关联
    // vaoID, attribIndex, bindingIndex
    // 这一步是关键，它告诉OpenGL：attribIndex 这个属性的数据来自 bindingIndex 这个绑定点
    glVertexArrayAttribBinding(id_, attribIndex, bindingIndex);

    // 4. 启用该顶点属性 (非常重要，否则着色器无法接收到数据)
    // vaoID, attribIndex
    glEnableVertexArrayAttrib(id_, attribIndex);
}

// 设置属性的步长（用于实例渲染）
void VertexArray::setBindingDivisor(GLuint bindingIndex, GLuint divisor)
{
    glVertexArrayBindingDivisor(id_, bindingIndex, divisor);
}

// 启用顶点属性 (DSA 方式)
void VertexArray::enableAttribute(GLuint attribIndex)
{
    glEnableVertexArrayAttrib(id_, attribIndex);
}

// 禁用顶点属性 (DSA 方式)
void VertexArray::disableAttribute(GLuint attribIndex)
{
    glDisableVertexArrayAttrib(id_, attribIndex);
}

// 绑定 IndexBuffer (DSA 方式)
void VertexArray::setIndexBuffer(const IndexBuffer &ibo)
{
    glVertexArrayElementBuffer(id_, ibo.id()); // DSA: 直接绑定 EBO 到 VAO
}

// 释放 VAO
void VertexArray::release()
{
    if (id_ != 0)
    {
        glDeleteVertexArrays(1, &id_);
        id_ = 0;
    }
}