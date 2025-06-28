#include "texture2D.h"
#include "debug_utils.h"
#include "glException.h" // For THROW_GL_EXCEPTION

// Include gli
#include <gli/gli.hpp>
#include <stdexcept> // For std::runtime_error
#include "stb_image.h"

// !!! 修改构造函数：现在接受 assetId，并将其传递给基类
// 私有构造函数 (供 TextureManager::internalLoadTexture2D/DDS 使用)
Texture2D::Texture2D(const std::string& assetId, GLenum target, GLenum internalFormat, int width, int height, int mipLevels)
    : Texture(target, internalFormat, width, height, 1, assetId, AssetType::Texture2D), // 传递 assetId 和 AssetType::Texture2D 给基类
      mipLevels_(mipLevels)
{
    // allocateStorage(mipLevels_);
    // setParameters();
}

// !!! 修改公有构造函数：现在接受 assetId，并将其传递给基类
// 公有构造函数 for general purpose (raw data upload or generation)
Texture2D::Texture2D(const std::string& assetId, int width, int height, GLenum internalFormat, int mipLevels)
    : Texture(GL_TEXTURE_2D, internalFormat, width, height, 1, assetId, AssetType::Texture2D), // 传递 assetId 和 AssetType::Texture2D 给基类
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
    GL_CHECK_ERROR();
}

void Texture2D::setParameters()
{
    glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, mipLevels_ > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CHECK_ERROR();
}

void Texture2D::uploadData(const void *data, GLenum format, GLenum type, int level)
{
    if (id_ == 0)
    {
        THROW_GL_EXCEPTION("Texture ID is 0. Cannot upload data.");
    }
    int currentWidth = std::max(1, width_ >> level);
    int currentHeight = std::max(1, height_ >> level);

    glTextureSubImage2D(id_, level, 0, 0, currentWidth, currentHeight, format, type, data); // DSA: 直接操作纹理ID上传数据
    GL_CHECK_ERROR();
}

// --- 移除静态工厂方法 loadDDS 和 loadFromFile 的实现 ---
// 这些逻辑将移至 TextureManager
/*
std::unique_ptr<Texture2D> Texture2D::loadDDS(const std::string &filePath) { ... }
std::unique_ptr<Texture2D> Texture2D::loadFromFile(const std::string &filePath, bool generateMipmaps, bool useSRGB) { ... }
*/