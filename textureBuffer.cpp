// TextureBuffer.cpp
#include "TextureBuffer.h"
#include <glad/glad.h>

TextureBuffer::TextureBuffer() : bufferId_(0), internalFormat_(GL_NONE)
{
    glGenTextures(1, &id_);
}

void TextureBuffer::bind() const
{
    glBindTexture(GL_TEXTURE_BUFFER, id_);
}

void TextureBuffer::unbind() const
{
    glBindTexture(GL_TEXTURE_BUFFER, 0);
}

void TextureBuffer::bindToUnit(GLuint unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_BUFFER, id_);
}

// ... (attachBuffer 和 release 不变)