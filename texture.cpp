// Texture.cpp (基类实现示例)
#include "Texture.h"
#include <glad/glad.h>
#include <stdexcept>
#include "debug_utils.h" // 用于 GL_CHECK_ERROR
#include "glException.h"

Texture::Texture(GLenum target, GLenum internalFormat, int width, int height, int depth)
    : target_(target), internalFormat_(internalFormat), width_(width), height_(height), depth_(depth)
{
    glCreateTextures(target_, 1, &id_); // DSA: 直接创建纹理对象
    if (id_ == 0) {
        THROW_GL_EXCEPTION("Failed to create texture ID for target " + std::to_string(target_) + ".");
    }
    GL_CHECK_ERROR(); // 检查是否有错误
}

void Texture::deleteGlResource() {
    if (id_ != 0) {
        glDeleteTextures(1, &id_);
        id_ = 0;
    }
}

// 这个方法用于激活纹理单元，以便在着色器中采样纹理
// void Texture::activate(GLenum textureUnit) const {
//     glActiveTexture(textureUnit); // 激活指定的纹理单元
//     glBindTexture(target_, id_); // 绑定纹理到其目标，以便采样器能找到它
//     GL_CHECK_ERROR(); // 检查是否有错误
// }

void Texture::activate(GLenum textureUnit) const {
    GLint unitIndex = textureUnit - GL_TEXTURE0;
    glBindTextureUnit(unitIndex, id_);
    GL_CHECK_ERROR();
}