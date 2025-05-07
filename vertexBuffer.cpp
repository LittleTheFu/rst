// VertexBuffer.cpp
#include "vertexBuffer.h"
#include <glad/glad.h>

VertexBuffer::VertexBuffer(const void* data, size_t size, GLenum usage) {
    generateId(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, id_);
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
}

VertexBuffer::~VertexBuffer() {
    release();
}

void VertexBuffer::bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, id_);
}

void VertexBuffer::unbind() const {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::updateData(const void* data, size_t size) {
    glBindBuffer(GL_ARRAY_BUFFER, id_);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW); // 默认使用 GL_STATIC_DRAW，可以根据需求修改
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::updateSubData(const void* data, size_t offset, size_t size) {
    glBindBuffer(GL_ARRAY_BUFFER, id_);
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::release() {
    if (id_ != 0) {
        glDeleteBuffers(1, &id_);
        id_ = 0;
    }
}