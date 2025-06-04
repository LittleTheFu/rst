#include "Texture2D.h"
#include "debug_utils.h"
#include "glException.h" // For THROW_GL_EXCEPTION

// Include gli
#include <gli/gli.hpp>
#include <stdexcept> // For std::runtime_error
#include "stb_image.h"

// Private constructor for use by static factory methods
Texture2D::Texture2D(GLenum target, GLenum internalFormat, int width, int height, int mipLevels)
    : Texture(target, internalFormat, width, height, 1), // depth is 1 for 2D
      mipLevels_(mipLevels)
{
}

// Public constructor for general purpose (raw data upload or generation)
Texture2D::Texture2D(int width, int height, GLenum internalFormat, int mipLevels)
    : Texture(GL_TEXTURE_2D, internalFormat, width, height, 1), // depth is 1 for 2D
      mipLevels_(mipLevels)
{
    allocateStorage(mipLevels_);
    setParameters();
}

void Texture2D::allocateStorage(int mipLevels)
{
    if (id_ == 0)
    {
        THROW_GL_EXCEPTION("Texture ID is 0. Cannot allocate storage.");
    }

    if (!glIsTexture(id_))
    {
        THROW_GL_EXCEPTION("Invalid OpenGL Texture ID before allocating storage.");
    }

    glTextureStorage2D(id_, mipLevels, internalFormat_, width_, height_);
}

void Texture2D::setParameters()
{
    // Note: For DDS, filter parameters might be derived from the file
    // or overridden here. For now, we set a default based on mip levels.
    glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, mipLevels_ > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CHECK_ERROR();

    // DDS textures usually come with pre-generated mipmaps, so we don't generate them here.
    // This block is mainly for textures loaded without mipmaps or generated on the fly.
    // However, if the DDS provides >1 mipLevel, GL_LINEAR_MIPMAP_LINEAR will work correctly.
    // glGenerateTextureMipmap(id_); // No need to call this if DDS already has mipmaps.
}

void Texture2D::uploadData(const void *data, GLenum format, GLenum type, int level)
{
    if (id_ == 0)
    {
        THROW_GL_EXCEPTION("Texture ID is 0. Cannot upload data.");
    }
    // Check if dimensions for this mip level are valid
    int currentWidth = std::max(1, width_ >> level);
    int currentHeight = std::max(1, height_ >> level);

    glTextureSubImage2D(id_, level, 0, 0, currentWidth, currentHeight, format, type, data); // DSA: 直接操作纹理ID上传数据
    GL_CHECK_ERROR();
}

// --- Static Factory Method for DDS Loading ---
std::unique_ptr<Texture2D> Texture2D::loadDDS(const std::string &filePath)
{
    gli::texture gli_texture = gli::load(filePath);

    if (gli_texture.empty() || gli_texture.target() != gli::TARGET_2D)
    {
        throw std::runtime_error("Failed to load 2D DDS texture from " + filePath + " or it's not a 2D texture.");
    }

    // Get texture properties from gli
    gli::texture::extent_type extent = gli_texture.extent(0); // Get dimensions of the base mip level

    gli::gl GLI_GL_Translator(gli::gl::PROFILE_GL33);
    gli::gl::format GLFormat = GLI_GL_Translator.translate(gli_texture.format(), gli::swizzles());
    GLenum internalFormat = static_cast<GLenum>(GLFormat.Internal);

    // GLenum internalFormat = gli::gl::internal_format(gli_texture.format());
    int width = extent.x;
    int height = extent.y;
    int mipLevels = static_cast<int>(gli_texture.levels());

    // Create a new Texture2D object using the private constructor
    // This constructor does NOT call allocateStorage or setParameters yet.
    std::unique_ptr<Texture2D> texture = std::make_unique<Texture2D>(
        GL_TEXTURE_2D, internalFormat, width, height, mipLevels);

    // Allocate storage for the texture based on DDS info
    texture->allocateStorage(mipLevels);

    // Upload data for all mip levels
    for (int level = 0; level < mipLevels; ++level)
    {
        gli::texture::extent_type level_extent = gli_texture.extent(level);
        GLsizei level_width = level_extent.x;
        GLsizei level_height = level_extent.y;

        // Determine if the format is compressed
        if (gli::is_compressed(gli_texture.format()))
        {
            // Compressed data upload
            GLsizei size = static_cast<GLsizei>(gli_texture.size(level));
            glCompressedTextureSubImage2D(texture->id(), level, 0, 0, level_width, level_height, internalFormat, size, gli_texture.data(0, 0, level));
            GL_CHECK_ERROR();
        }
        else
        {
            // Uncompressed data upload
            GLenum format = gli::gl::external_format(gli_texture.format());
            GLenum type = gli::gl::type_format(gli_texture.format());
            texture->uploadData(gli_texture.data(0, 0, level), format, type, level);
        }
    }

    // Set texture parameters. We can use the existing setParameters,
    // but ensure it's compatible with DDS expectations (e.g., mipmaps already present).
    // The existing setParameters will handle GL_LINEAR_MIPMAP_LINEAR if mipLevels > 1.
    texture->setParameters();

    return texture;
}

std::unique_ptr<Texture2D> Texture2D::loadFromFile(const std::string &filePath, bool generateMipmaps)
{
    int width, height, numChannels;
    
    // STBI_rgb_alpha 强制加载为 RGBA (4 个通道)，即使原图是 RGB
    // 这是为了确保数据布局一致性，避免 OpenGL 格式匹配问题
    unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &numChannels, 4);

    if (!data)
    {
        throw std::runtime_error("Failed to load image from " + filePath + ": " + stbi_failure_reason());
    }

    // 确定 OpenGL 内部格式 (假设我们总是将其视为 RGBA8)
    GLenum internalFormat = GL_RGBA8;
    // GLenum internalFormat = GL_RGBA8;
    // GLenum format = GL_RGBA; // external format for upload
    // GLenum type = GL_UNSIGNED_BYTE; // data type for upload

    // 计算 Mipmap 级别数量
    int mipLevels = 1;
    if (generateMipmaps)
    {
        // 计算最大可能的 mipmap 级别
        mipLevels = static_cast<int>(std::floor(std::log2(std::max(width, height)))) + 1;
    }

    // 创建 Texture2D 对象 (使用私有构造函数)
    // 确保 Texture::Texture 构造函数中 glCreateTextures 的 target 是 GL_TEXTURE_2D
    std::unique_ptr<Texture2D> texture = std::make_unique<Texture2D>(
        width, height, internalFormat, mipLevels);

    // 分配存储空间
    // texture->allocateStorage(mipLevels);

    // 上传基础 mip 级别的数据
    texture->uploadData(data, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    // 如果需要，生成 Mipmap
    if (generateMipmaps && mipLevels > 1)
    {
        glGenerateTextureMipmap(texture->id()); // DSA: 直接生成指定纹理的 mipmap
        GL_CHECK_ERROR();
    }

    // 设置纹理参数 (包括 mipmap 过滤模式)
    // texture->setParameters(); // setParameters 内部会根据 mipLevels_ 判断过滤模式

    // 释放 stbi_load 分配的内存
    stbi_image_free(data);

    return texture;
}