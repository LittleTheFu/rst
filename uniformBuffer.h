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

    void create(size_t size, GLenum usage);

    // 实现 GLResource::bind() 纯虚函数，用于通用绑定
    void bind() const override;

    // 实现 GLResource::unbind() 纯虚函数
    void unbind() const override;

    // 新增方法：将UBO绑定到特定的着色器绑定点
    void bindToBindingPoint(GLuint bindingPoint) const;

    void updateData(size_t offset, size_t size, const void* data);

protected:
    void release() override;

private:
    size_t size_ = 0;
};

#endif // UNIFORM_BUFFER_H