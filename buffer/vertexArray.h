#ifndef VERTEXARRAY_H
#define VERTEXARRAY_H

#include <glad/glad.h>
#include <memory>
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "GLResource.h" // 假设 GLResource 是你的基类

class VertexArray : public GLResource
{
public:
    VertexArray();
    VertexArray(VertexArray &&other) noexcept;
    VertexArray &operator=(VertexArray &&other) noexcept;

    void bind() const;
    void unbind() const;

    // 修改 setAttribute 函数签名，添加 isInteger 参数
    void setAttribute(
        GLuint attribIndex,
        const VertexBuffer &vbo,
        GLint size,
        GLenum type,
        GLboolean normalized,
        GLuint relativeOffset,
        GLsizei stride,
        GLuint bindingIndex,
        bool isInteger = false); // <-- 新增 isInteger 参数，并提供默认值

    void setBindingDivisor(GLuint bindingIndex, GLuint divisor);
    void enableAttribute(GLuint attribIndex);
    void disableAttribute(GLuint attribIndex);
    void setIndexBuffer(const IndexBuffer &ibo);

private:
    void release() override; // 继承自 GLResource
};

#endif // VERTEXARRAY_H