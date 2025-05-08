// TextureBuffer.h
#ifndef TEXTURE_BUFFER_H
#define TEXTURE_BUFFER_H

#include "GLResource.h"
#include <glad/glad.h>

class TextureBuffer : public GLResource {
public:
    TextureBuffer();
    ~TextureBuffer() override;

    void bind() const override; // 正确：与基类签名一致
    void unbind() const override;

    void attachBuffer(GLuint bufferId, GLenum internalFormat);
    void bindToUnit(GLuint unit) const; // 用于绑定到特定纹理单元

private:
    GLuint bufferId_;
    GLenum internalFormat_;
    void release() override;
};

#endif // TEXTURE_BUFFER_H