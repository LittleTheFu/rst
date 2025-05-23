#include "TextureCubeMap.h"
#include <iostream>
// #include <glm/glm.hpp> // 这一行可以删除，因为它不再需要了
#include <gli/gli.hpp> // 这一行必须保留！

TextureCubeMap::TextureCubeMap() : id_(0), sideLength_(0), mipLevels_(0)
{
    glGenTextures(1, &id_);
}

TextureCubeMap::~TextureCubeMap()
{
    if (id_ != 0) {
        glDeleteTextures(1, &id_);
    }
}

bool TextureCubeMap::loadDDS(const std::string& path)
{
    // 使用 gli::load 加载 DDS 文件
    gli::texture Texture = gli::load(path);
    if (Texture.empty()) {
        std::cerr << "ERROR::TEXTURE_CUBEMAP::Failed to load DDS texture: " << path << std::endl;
        return false;
    }

    // 检查加载的纹理是否确实是立方体贴图
    if (Texture.target() != gli::TARGET_CUBE) {
        std::cerr << "ERROR::TEXTURE_CUBEMAP::DDS file is not a cubemap: " << path << std::endl;
        return false;
    }

    sideLength_ = Texture.extent(0).x; // 获取 mip level 0 的宽度 (立方体贴图宽高相等)
    mipLevels_ = Texture.levels();      // 获取 mipmap 级别数量

    // 绑定 OpenGL 纹理
    glBindTexture(GL_TEXTURE_CUBE_MAP, id_);

    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); // 立方体贴图需要 GL_TEXTURE_WRAP_R

    // 如果包含 mipmap，使用线性 mipmap 过滤；否则使用线性过滤
    if (mipLevels_ > 1) {
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 获取 gli 格式对应的 OpenGL 内部格式、外部格式和数据类型
    // gli::internal_format, gli::external_format, gli::type_format 都是 gli 库提供的函数
    // 获取 gli 格式对应的 OpenGL 内部格式、外部格式和数据类型
    // 这些函数位于 gli::gl 命名空间下
    GLenum internalFormat = static_cast<GLenum>(gli::gl::internal_format(Texture.format()));
    GLenum format = static_cast<GLenum>(gli::gl::external_format(Texture.format()));
    GLenum type = static_cast<GLenum>(gli::gl::type_format(Texture.format()));

    // 循环遍历所有 mipmap 级别和所有立方体贴图的面，上传数据
    // gli::texture::data(face, layer, level)
    // Cubemaps have 6 faces, layer is always 0 for non-array cubemaps
    for (int face = 0; face < 6; ++face) {
        for (int mip = 0; mip < mipLevels_; ++mip) {
            // 获取当前 mipmap 级别的尺寸
            gli::extent3d extent = Texture.extent(mip);
            
            // 将数据上传到 OpenGL
            // GL_TEXTURE_CUBE_MAP_POSITIVE_X + face 对应各个面
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                         mip,
                         internalFormat,
                         extent.x, // 宽度
                         extent.y, // 高度
                         0,        // border (always 0)
                         format,
                         type,
                         Texture.data(face, 0, mip)); // 获取当前面和 mipmap 级别的数据指针
        }
    }

    // 解绑纹理
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return true;
}

void TextureCubeMap::use(unsigned int slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id_);
}