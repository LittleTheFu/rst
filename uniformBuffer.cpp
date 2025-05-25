#include "UniformBuffer.h"
#include <glad/glad.h>
#include <iostream>

UniformBuffer::UniformBuffer()
{
}

void UniformBuffer::create(size_t size, GLenum usage)
{
    size_ = size;

    glGenBuffers(1, &id_); // id_ 是 GLResource 的保护成员，可以直接访问
    
    // 绑定用于初始数据分配
    glBindBuffer(GL_UNIFORM_BUFFER, id_);
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, usage);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

// 实现 GLResource::bind()：用于通用操作，如更新数据
void UniformBuffer::bind() const
{
    glBindBuffer(GL_UNIFORM_BUFFER, id_);
}

// 实现 GLResource::unbind()
void UniformBuffer::unbind() const
{
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

// 新增方法：将UBO绑定到特定的着色器绑定点
void UniformBuffer::bindToBindingPoint(GLuint bindingPoint) const
{
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, id_);
}

void UniformBuffer::updateData(size_t offset, size_t size, const void* data)
{
    // 这里调用 bind() 来确保缓冲区已绑定
    bind(); // 使用统一的 bind() 方法
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
    unbind(); // 使用统一的 unbind() 方法
}

void UniformBuffer::release()
{
    if (id_ != 0) { // 直接访问 id_ 成员变量
        glDeleteBuffers(1, &id_);
        id_ = 0;
        size_ = 0;
    }
}