#include "UniformBuffer.h"
#include <iostream> 

UniformBuffer::UniformBuffer() 
{ 
} 

void UniformBuffer::create(size_t size, GLenum usage) 
{ 
    size_ = size; 
    // DSA：直接创建缓冲区对象
    glCreateBuffers(1, &id_); 
    // DSA：直接为指定 ID 的缓冲区分配存储空间
    glNamedBufferData(id_, size, nullptr, usage); 
} 

// 移除了 bind() 和 unbind() 方法，因为它们不再是 DSA 风格操作所必需的。

void UniformBuffer::bindToBindingPoint(GLuint bindingPoint) const 
{ 
    // 此函数仍然至关重要，因为它将 UBO 连接到着色器的 uniform 块。
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, id_); 
} 

void UniformBuffer::updateData(size_t offset, size_t size, const void* data) 
{ 
    // DSA：直接更新指定 ID 的缓冲区对象的数据，无需绑定。
    glNamedBufferSubData(id_, offset, size, data); 
} 

void UniformBuffer::release() 
{ 
    if (id_ != 0) {
        glDeleteBuffers(1, &id_); 
        id_ = 0; 
        size_ = 0; 
    } 
}