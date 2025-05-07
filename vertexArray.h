// VertexArray.h
#ifndef VERTEX_ARRAY_H
#define VERTEX_ARRAY_H

#include "GLResource.h"
#include <glad/glad.h>

class VertexArray : public GLResource {
public:
    VertexArray();
    ~VertexArray() override;

    void bind() const override;
    void unbind() const override;

private:
    void release() override;
};

#endif // VERTEX_ARRAY_H