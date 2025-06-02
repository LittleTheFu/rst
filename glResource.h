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

    // --- 禁用拷贝构造函数和拷贝赋值运算符 --- 
    // 防止资源共享，强制使用移动语义 
    GLResource(const GLResource&) = delete;
    GLResource& operator=(const GLResource&) = delete;

protected:
    // GLuint generateId(GLenum target); // 根据目标类型生成 ID 
    virtual void release() = 0;      // 子类实现资源释放逻辑 

    GLuint id_;
};

#endif // GL_RESOURCE_H