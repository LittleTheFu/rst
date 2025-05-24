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
    if (id_ != 0)
    {
        glDeleteTextures(1, &id_);
    }
}

bool TextureCubeMap::loadDDS(const std::string &path)
{
    // 使用 gli::load 加载 DDS 文件
    gli::texture Texture = gli::load(path);
    if (Texture.empty())
    {
        std::cerr << "ERROR::TEXTURE_CUBEMAP::Failed to load DDS texture: " << path << std::endl;
        return false;
    }

    // 检查加载的纹理是否确实是立方体贴图
    if (Texture.target() != gli::TARGET_CUBE)
    {
        std::cerr << "ERROR::TEXTURE_CUBEMAP::DDS file is not a cubemap: " << path << std::endl;
        return false;
    }

    sideLength_ = Texture.extent(0).x; // 获取 mip level 0 的宽度 (立方体贴图宽高相等)
    mipLevels_ = Texture.levels();     // 获取 mipmap 级别数量

    // 绑定 OpenGL 纹理
    glBindTexture(GL_TEXTURE_CUBE_MAP, id_);

    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); // 立方体贴图需要 GL_TEXTURE_WRAP_R

    // 如果包含 mipmap，使用线性 mipmap 过滤；否则使用线性过滤
    if (mipLevels_ > 1)
    {
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // *** MODIFIED START ***
    // 1. 创建 gli::gl 类的实例。通常选择一个 profile，例如 PROFILE_GL33。
    //    这里的 gli::gl::PROFILE_GL33 是 gli/gl.hpp 中定义的枚举值
    gli::gl GLI_GL_Translator(gli::gl::PROFILE_GL33);

    // 2. 调用 translate 函数来获取 gl::format 结构体
    //    对于DDS文件，swizzle通常由文件自身定义，或者默认使用RGBA。
    //    这里我们传递一个默认的swizzle (Identity)
    gli::gl::format GLFormat = GLI_GL_Translator.translate(Texture.format(), gli::swizzles());

    // 3. 从返回的结构体中提取 OpenGL 枚举
    GLenum internalFormat = static_cast<GLenum>(GLFormat.Internal);
    GLenum format = static_cast<GLenum>(GLFormat.External);
    GLenum type = static_cast<GLenum>(GLFormat.Type);
    // *** MODIFIED END ***

    std::cout << "DEBUG::TEXTURE_CUBEMAP::Loading " << path << std::endl;
    std::cout << "DEBUG::  Texture.format(): " << Texture.format() << std::endl; // gli内部格式枚举值
    std::cout << "DEBUG::  internalFormat (GLenum): " << internalFormat << " (0x" << std::hex << internalFormat << std::dec << ")" << std::endl;
    std::cout << "DEBUG::  format (GLenum): " << format << " (0x" << std::hex << format << std::dec << ")" << std::endl;
    std::cout << "DEBUG::  type (GLenum): " << type << " (0x" << std::hex << type << std::dec << ")" << std::endl;
    std::cout << "DEBUG::  mipLevels_: " << mipLevels_ << std::endl;
    std::cout << "DEBUG::  sideLength_: " << sideLength_ << std::endl;

    // 检查纹理格式是否被压缩
    bool isCompressed = gli::is_compressed(Texture.format()); // 需要 #include <gli/format.hpp> 或者 <gli/gli.hpp>

    for (int face = 0; face < 6; ++face)
    {
        for (int mip = 0; mip < mipLevels_; ++mip)
        {
            gli::extent3d extent = Texture.extent(mip);

            if (extent.x <= 0 || extent.y <= 0)
            {
                std::cerr << "ERROR::TEXTURE_CUBEMAP::Invalid extent for " << path
                          << " face " << face << " mip " << mip << ": "
                          << extent.x << "x" << extent.y << std::endl;
                return false;
            }

            // 获取当前 mip 级别和面的图像数据大小
            // 对于压缩纹理，gli::texture::size() 返回指定 mip 和面的压缩数据大小。
            // 对于非压缩纹理，它返回宽度 * 高度 * 每像素字节数。
            GLsizei imageSize = static_cast<GLsizei>(Texture.size(mip));

            if (isCompressed)
            {
                glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                                       mip,
                                       internalFormat, // 这是压缩内部格式（例如，GL_COMPRESSED_RGBA_BPTC_UNORM）
                                       extent.x,
                                       extent.y,
                                       0,                           // border，始终为 0
                                       imageSize,                   // 压缩数据的大小
                                       Texture.data(0, face, mip)); // 指向压缩数据的指针
            }
            else
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                             mip,
                             internalFormat,
                             extent.x,
                             extent.y,
                             0,
                             format, // 对于非压缩格式，这将是非零值
                             type,   // 对于非压缩格式，这将是非零值
                             Texture.data(0, face, mip));
            }

            int error = glGetError();
            if (error != GL_NO_ERROR)
            {
                std::cerr << "OpenGL Error after glTexImage2D/glCompressedTexImage2D for " << path
                          << " face " << face << " mip " << mip << ": " << error
                          << " (0x" << std::hex << error << std::dec << ")" << std::endl;
                return false;
            }
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