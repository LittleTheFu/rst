#ifndef UNIFORM_BUFFER_H
#define UNIFORM_BUFFER_H

#include "GLResource.h"
#include <glad/glad.h>
#include <string>
#include <vector>

class UniformBuffer : public GLResource
{
public:
    UniformBuffer();
    ~UniformBuffer() override;

    void create(size_t size, GLenum usage);
    void bind(GLuint bindingPoint) const override;
    void unbind() const override;
    void updateData(size_t offset, size_t size, const void* data);

protected:
    void release() override;

private:
    size_t size_ = 0;
};

#endif // UNIFORM_BUFFER_H