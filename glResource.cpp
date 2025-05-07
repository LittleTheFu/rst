// GLResource.cpp
#include "GLResource.h"
#include <glad/glad.h>

GLuint GLResource::generateId(GLenum target) {
    GLuint newId = 0;
    switch (target) {
        case GL_ARRAY_BUFFER:
        case GL_ELEMENT_ARRAY_BUFFER:
            glGenBuffers(1, &newId);
            break;
        case GL_FRAMEBUFFER:
            glGenFramebuffers(1, &newId);
            break;
        case GL_VERTEX_ARRAY:
            glGenVertexArrays(1, &newId);
            break;
        case GL_TEXTURE:
            glGenTextures(1, &newId);
            break;
        case GL_RENDERBUFFER:
            glGenRenderbuffers(1, &newId);
            break;
        // 可以根据需要添加其他 OpenGL 对象类型
        default:
            // 处理不支持的类型，可以抛出异常或记录错误
            break;
    }
    id_ = newId;
    return id_;
}

void GLResource::release() {
    // 基类的 release 不做任何操作，由子类根据其资源类型实现
}