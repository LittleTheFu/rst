// GLResource.h
#ifndef GL_RESOURCE_H
#define GL_RESOURCE_H

#include <glad/glad.h>

class GLResource {
public:
    GLResource() : id_(0) {}
    virtual ~GLResource() { release(); }

    GLuint id() const { return id_; }

    virtual void bind() const = 0;   // 纯虚函数，强制子类实现
    virtual void unbind() const = 0; // 纯虚函数，强制子类实现

protected:
    GLuint generateId(GLenum target); // 根据目标类型生成 ID
    virtual void release() = 0;      // 子类实现资源释放逻辑

    GLuint id_;
};

#endif // GL_RESOURCE_H