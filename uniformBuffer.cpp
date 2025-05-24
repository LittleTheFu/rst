#include "UniformBuffer.h"
#include <glad/glad.h>
#include <iostream>

UniformBuffer::UniformBuffer() : GLResource()
{
}

UniformBuffer::~UniformBuffer()
{
    // release() 会在基类的析构函数中被调用
}

void UniformBuffer::create(size_t size, GLenum usage)
{
    size_ = size;

    glGenBuffers(1, &id_);
    
    glBindBuffer(GL_UNIFORM_BUFFER, id());
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, usage);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::bind(GLuint bindingPoint) const
{
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, id());
}

void UniformBuffer::unbind() const
{
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::updateData(size_t offset, size_t size, const void* data)
{
    glBindBuffer(GL_UNIFORM_BUFFER, id());
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::release()
{
    if (id() != 0) {
        glDeleteBuffers(1, &id_);
        id_ = 0;
        size_ = 0;
    }
}