// Texture2D.cpp
#include "Texture2D.h"
#include "debug_utils.h"

Texture2D::Texture2D(int width, int height, GLenum internalFormat, int mipLevels)
    : Texture(GL_TEXTURE_2D, internalFormat, width, height, 1), // depth is 1 for 2D
      mipLevels_(mipLevels)
{
    // 在构造函数中完成分配和参数设置，使对象一经创建就可用
    allocateStorage(mipLevels_);
    setParameters();
}

void Texture2D::allocateStorage(int mipLevels) {
    glTextureStorage2D(id_, mipLevels, internalFormat_, width_, height_); // DSA: 直接操作纹理ID
    GL_CHECK_ERROR();
}

void Texture2D::setParameters() {
    glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, mipLevels_ > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CHECK_ERROR();

    if (mipLevels_ > 1) {
        glGenerateTextureMipmap(id_); // DSA: 直接为纹理ID生成Mipmap
        GL_CHECK_ERROR();
    }
}

void Texture2D::uploadData(const void* data, GLenum format, GLenum type, int level) {
    glTextureSubImage2D(id_, level, 0, 0, width_, height_, format, type, data); // DSA: 直接操作纹理ID上传数据
    GL_CHECK_ERROR();
}