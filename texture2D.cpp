#include "Texture2D.h"
#include <iostream>
#include <gli/gli.hpp>   // 确保包含此文件以获取 gli::load 和 gli::is_compressed
#include <gli/gl.hpp>    // 确保包含此文件以获取 gli::gl 类

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
    if (Texture.target() != gli::TARGET_2D) {
        std::cerr << "ERROR::TEXTURE_2D::DDS file is not a 2D texture: " << path << std::endl;
        return false;
    }

    width_ = Texture.extent(0).x;   // 获取 mip level 0 的宽度
    height_ = Texture.extent(0).y;  // 获取 mip level 0 的高度
    mipLevels_ = Texture.levels(); // 获取 mipmap 级别数量

    // 绑定 OpenGL 纹理
    glBindTexture(GL_TEXTURE_2D, id_);
    GLenum error = glGetError(); // 检查绑定错误
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL Error after glBindTexture for " << path << ": " << error << std::endl;
        return false;
    }

    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    error = glGetError(); // 检查参数设置错误
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL Error after glTexParameteri for " << path << ": " << error << std::endl;
        return false;
    }

    if (mipLevels_ > 1) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    error = glGetError(); // 检查过滤设置错误
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL Error after setting min/mag filter for " << path << ": " << error << std::endl;
        return false;
    }

    // *** 关键修改开始 ***
    // 1. 创建 gli::gl 类的实例，通常选择一个 profile，例如 PROFILE_GL33。
    gli::gl GLI_GL_Translator(gli::gl::PROFILE_GL33); 
    
    // 2. 调用 translate 函数来获取 gl::format 结构体
    // 对于 BRDF LUT，通常是浮点格式（例如 R16G16_SFLOAT），通道通常是 RG。
    // 如果不确定，可以先用 gli::SWIZZLE_RGBA，gli会根据文件格式推断
    gli::gl::format GLFormat = GLI_GL_Translator.translate(Texture.format(), gli::swizzles()); 
    
    // 3. 从返回的结构体中提取 OpenGL 枚举
    GLenum internalFormat = static_cast<GLenum>(GLFormat.Internal);
    GLenum format = static_cast<GLenum>(GLFormat.External);
    GLenum type = static_cast<GLenum>(GLFormat.Type);
    // *** 关键修改结束 ***

    // 打印出这些值，以便调试
    std::cout << "DEBUG::TEXTURE_2D::Loading " << path << std::endl;
    std::cout << "DEBUG::  Texture.format(): " << Texture.format() << std::endl; // gli内部格式枚举值
    std::cout << "DEBUG::  internalFormat (GLenum): " << internalFormat << " (0x" << std::hex << internalFormat << std::dec << ")" << std::endl;
    std::cout << "DEBUG::  format (GLenum): " << format << " (0x" << std::hex << format << std::dec << ")" << std::endl;
    std::cout << "DEBUG::  type (GLenum): " << type << " (0x" << std::hex << type << std::dec << ")" << std::endl;
    std::cout << "DEBUG::  width_: " << width_ << std::endl;
    std::cout << "DEBUG::  height_: " << height_ << std::endl;
    std::cout << "DEBUG::  mipLevels_: " << mipLevels_ << std::endl;

    // 检查纹理格式是否被压缩
    bool isCompressed = gli::is_compressed(Texture.format());

    // 循环遍历所有 mipmap 级别
    for (int mip = 0; mip < mipLevels_; ++mip) {
        // 获取当前 mipmap 级别的尺寸
        gli::extent3d extent = Texture.extent(mip);
        
        if (extent.x <= 0 || extent.y <= 0) {
            std::cerr << "ERROR::TEXTURE_2D::Invalid extent for " << path 
                      << " mip " << mip << ": " 
                      << extent.x << "x" << extent.y << std::endl;
            return false;
        }

        GLsizei imageSize = static_cast<GLsizei>(Texture.size(mip)); 

        // 将数据上传到 OpenGL
        if (isCompressed) {
            glCompressedTexImage2D(GL_TEXTURE_2D,
                                 mip,
                                 internalFormat,
                                 extent.x,
                                 extent.y,
                                 0, // border (always 0)
                                 imageSize, // Size of the compressed data
                                 Texture.data(0, 0, mip)); // 对于 2D 纹理，face 和 layer 都是 0
        } else {
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

        error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "OpenGL Error after glTexImage2D/glCompressedTexImage2D for " << path 
                      << " mip " << mip << ": " << error 
                      << " (0x" << std::hex << error << std::dec << ")" << std::endl;
            return false;
        }
    }

    // 解绑纹理
    glBindTexture(GL_TEXTURE_2D, 0);
    error = glGetError(); // 检查解绑错误
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL Error after unbinding texture for " << path << ": " << error << std::endl;
    }

    return true;
}

void Texture2D::use(unsigned int slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, id_);
}