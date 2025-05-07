// VertexArray.cpp
#include "vertexArray.h"
#include <glad/glad.h>

VertexArray::VertexArray() {
    generateId(GL_VERTEX_ARRAY);
}

VertexArray::~VertexArray() {
    release();
}

void VertexArray::bind() const {
    glBindVertexArray(id_);
}

void VertexArray::unbind() const {
    glBindVertexArray(0);
}

void VertexArray::release() {
    if (id_ != 0) {
        glDeleteVertexArrays(1, &id_);
        id_ = 0;
    }
}