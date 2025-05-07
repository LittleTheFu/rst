// VertexBuffer.h
#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#include "GLResource.h"
#include <glad/glad.h>

class VertexBuffer : public GLResource {
public:
    VertexBuffer(const void* data, size_t size, GLenum usage);
    ~VertexBuffer() override;

    void bind() const override;
    void unbind() const override;

    void updateData(const void* data, size_t size);
    void updateSubData(const void* data, size_t offset, size_t size);

private:
    void release() override;
};

#endif // VERTEX_BUFFER_H