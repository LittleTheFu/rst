// IndexBuffer.h
#ifndef INDEX_BUFFER_H
#define INDEX_BUFFER_H

#include "GLResource.h"
#include <glad/glad.h>

class IndexBuffer : public GLResource {
public:
    IndexBuffer(const unsigned int* data, size_t count, GLenum usage);
    ~IndexBuffer() override;

    void bind() const override;
    void unbind() const override;

    size_t getCount() const;

private:
    size_t count_;
    void release() override;
};

#endif // INDEX_BUFFER_H