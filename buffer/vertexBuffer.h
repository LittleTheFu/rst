#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#include "GLResource.h"
#include <glad/glad.h>
#include <cstddef> // For size_t

class VertexBuffer : public GLResource {
public:
    // 构造函数：直接使用 DSA 函数创建和初始化缓冲区
    // 不再需要 bind()/unbind() 相关的虚函数
    VertexBuffer(const void* data, GLsizeiptr size, GLenum usage); // 使用 GLsizeiptr 更符合OpenGL API

    // 默认构造函数（可选）：如果需要先创建再填充数据
    VertexBuffer(); 

    // 上传所有数据（重新分配存储）
    void setData(const void* data, GLsizeiptr size, GLenum usage);

    // 更新部分数据（不重新分配存储）
    void updateSubData(const void* data, GLintptr offset, GLsizeiptr size);

    // 映射缓冲区到内存
    void* mapBuffer(GLenum access);
    // 解映射缓冲区
    void unmapBuffer();

    // 禁用复制构造函数和赋值运算符，防止资源共享导致的 double free
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    // 移动构造函数和移动赋值运算符（推荐实现）
    VertexBuffer(VertexBuffer&& other) noexcept;
    VertexBuffer& operator=(VertexBuffer&& other) noexcept;

protected:
    // 实现 GLResource 的 release 纯虚函数
    void release() override;
};

#endif // VERTEX_BUFFER_H