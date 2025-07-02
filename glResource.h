// GLResource.h
#ifndef GL_RESOURCE_H
#define GL_RESOURCE_H

#include <glad/glad.h>

class GLResource {
public:
    GLResource();
    virtual ~GLResource();

    GLuint id() const;

    GLResource(GLResource&& other) noexcept;
    GLResource& operator=(GLResource&& other) noexcept;

    GLResource(const GLResource&) = delete;
    GLResource& operator=(const GLResource&) = delete;

protected:
    virtual void release() = 0;

    GLuint id_;
};

#endif