#include "Texture2D.h"
#include <iostream>

Texture2D::Texture2D() : id_(0), width_(0), height_(0), mipLevels_(0)
{
    glGenTextures(1, &id_);
}

Texture2D::~Texture2D()
{
    if (id_ != 0) {
        glDeleteTextures(1, &id_);
    }
}

bool Texture2D::loadDDS(const std::string& path)
{
    // 使用 gli::load 加载 DDS 文件
    gli::texture Texture = gli::load(path);
    if (Texture.empty()) {
        std::cerr << "ERROR::TEXTURE_2D::Failed to load DDS texture: " << path << std::endl;
        return false;
    }

    // 检查加载的纹理是否确实是 2D 纹理
    // 对于 BRDF LUT，它应该是 gli::TARGET_2D
    if (Texture.target() != gli::TARGET_2D) {
        std::cerr << "ERROR::TEXTURE_2D::DDS file is not a 2D texture: " << path << std::endl;
        return false;
    }

    width_ = Texture.extent(0).x;  // 获取 mip level 0 的宽度
    height_ = Texture.extent(0).y; // 获取 mip level 0 的高度
    mipLevels_ = Texture.levels(); // 获取 mipmap 级别数量

    // 绑定 OpenGL 纹理
    glBindTexture(GL_TEXTURE_2D, id_);

    // 设置纹理参数
    // BRDF LUT 通常使用 GL_CLAMP_TO_EDGE，因为它是查找表
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // BRDF LUT 通常不需要 mipmaps，但也可能包含。
    // 如果包含 mipmap，使用线性 mipmap 过滤；否则使用线性过滤
    if (mipLevels_ > 1) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 获取 gli 格式对应的 OpenGL 内部格式、外部格式和数据类型
    GLenum internalFormat = static_cast<GLenum>(gli::gl::internal_format(Texture.format()));
    GLenum format = static_cast<GLenum>(gli::gl::external_format(Texture.format()));
    GLenum type = static_cast<GLenum>(gli::gl::type_format(Texture.format()));

    // 循环遍历所有 mipmap 级别（对于 2D 纹理，face 和 layer 都是 0）
    for (int mip = 0; mip < mipLevels_; ++mip) {
        // 获取当前 mipmap 级别的尺寸
        gli::extent3d extent = Texture.extent(mip);
        
        // 将数据上传到 OpenGL
        // gli::texture::data(face, layer, level)
        // 对于 2D 纹理，face 和 layer 都是 0
        glTexImage2D(GL_TEXTURE_2D,
                     mip,
                     internalFormat,
                     extent.x, // 宽度
                     extent.y, // 高度
                     0,        // border (always 0)
                     format,
                     type,
                     Texture.data(0, 0, mip)); // 获取当前 mipmap 级别的数据指针
    }

    // 解绑纹理
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

void Texture2D::use(unsigned int slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, id_);
}