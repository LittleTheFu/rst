#include "texture.h"
#include <glad/glad.h>
#include <stdexcept>
#include "debug_utils.h" // 用于 GL_CHECK_ERROR
#include "glException.h"

// !!! 修改构造函数签名以匹配 Texture.h
Texture::Texture(GLenum target, GLenum internalFormat, int width, int height, int depth,
                 const std::string& assetId, AssetType assetType)
    : GLResource(), // 调用 GLResource 的默认构造函数
      target_(target), internalFormat_(internalFormat), width_(width), height_(height), depth_(depth),
      m_id(assetId), m_type(assetType) // 初始化 IAsset 成员
{
    glCreateTextures(target_, 1, &id_); // DSA: 直接创建纹理对象
    if (id_ == 0) {
        THROW_GL_EXCEPTION("Failed to create texture ID for target " + std::to_string(target_) + ".");
    }
    GL_CHECK_ERROR(); // 检查是否有错误
}

// 实际释放 OpenGL 纹理资源的方法
void Texture::release() {
    if (id_ != 0) {
        glDeleteTextures(1, &id_);
        id_ = 0; // 清空 ID，防止二次释放
    }
    GL_CHECK_ERROR(); // 检查是否有错误
}

void Texture::activate(GLenum textureUnit) const {
    glBindTextureUnit(textureUnit, id_);
    GL_CHECK_ERROR();
}