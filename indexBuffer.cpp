// IndexBuffer.cpp
#include "indexBuffer.h"
#include <glad/glad.h>

IndexBuffer::IndexBuffer(const unsigned int* data, size_t count, GLenum usage)
    : count_(count) {
    generateId(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, usage);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

IndexBuffer::~IndexBuffer() {
    release();
}

void IndexBuffer::bind() const override {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_);
}

void IndexBuffer::unbind() const override {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

size_t IndexBuffer::getCount() const {
    return count_;
}

void IndexBuffer::release() override {
    if (id_ != 0) {
        glDeleteBuffers(1, &id_);
        id_ = 0;
    }
}