// TextureCubeMap.cpp
#include "TextureCubeMap.h"
#include "debug_utils.h"

TextureCubeMap::TextureCubeMap(int resolution, GLenum internalFormat, int mipLevels)
    : Texture(GL_TEXTURE_CUBE_MAP, internalFormat, resolution, resolution, 1), // resolution is width/height
      mipLevels_(mipLevels)
{
    allocateStorage(mipLevels_);
    setParameters();
}

void TextureCubeMap::allocateStorage(int mipLevels) {
    glTextureStorage2D(id_, mipLevels, internalFormat_, width_, height_); // DSA: 对 Cubemap ID 分配所有面的存储
    GL_CHECK_ERROR();
}

void TextureCubeMap::setParameters() {
    glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, mipLevels_ > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); // Cubemap 独有
    GL_CHECK_ERROR();

    if (mipLevels_ > 1) {
        glGenerateTextureMipmap(id_);
        GL_CHECK_ERROR();
    }
}

void TextureCubeMap::uploadFaceData(GLenum faceTarget, const void* data, GLenum format, GLenum type, int level) {
    // 这里的 faceTarget 应该是 GL_TEXTURE_CUBE_MAP_POSITIVE_X 等
    // 注意：glTextureSubImage2D 的第一个参数是纹理 ID，不是目标。
    // 但是对于上传 Cubemap 单个面的数据，你需要使用 glTexSubImage2D 并在其第一个参数传入 faceTarget。
    // 这意味着这里需要一个临时绑定：
    // 或者，对于 DSA，glTextureSubImage2D 是不支持直接指定 Cubemap 面的，它只用于 2D。
    // 这是一个 DSA 的尴尬点：上传 Cubemap 单个面数据，仍然需要 glBindTexture。
    // 解决办法：如果你的纹理对象在创建后是完全由 GPU 管理的（如 glTextureStorage2D），
    // 并且不打算从 CPU 上传单个面数据，那么你可以完全避免绑定。
    // 如果需要，就得这么做：
    glBindTexture(target_, id_); // 临时绑定纹理
    glTexSubImage2D(faceTarget, level, 0, 0, width_, height_, format, type, data);
    glBindTexture(target_, 0); // 解绑
    GL_CHECK_ERROR();
}