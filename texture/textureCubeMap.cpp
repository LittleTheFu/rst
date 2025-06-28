#include "textureCubeMap.h"
#include "debug_utils.h"
#include "glException.h" // For THROW_GL_EXCEPTION

// Include gli
#include <gli/gli.hpp>
#include <stdexcept> // For std::runtime_error

// !!! 修改构造函数：接受 assetId 并传递给基类
// 私有构造函数，供 TextureManager 内部使用
TextureCubeMap::TextureCubeMap(const std::string& assetId, GLenum target, GLenum internalFormat, int resolution, int mipLevels)
    : Texture(target, internalFormat, resolution, resolution, 1, assetId, AssetType::TextureCubeMap), // 传递 assetId 和 AssetType::TextureCubeMap 给基类
      mipLevels_(mipLevels)
{
}

// !!! 修改公有构造函数：接受 assetId 并传递给基类
// 公有构造函数，用于通用目的（原始数据上传或运行时生成）
TextureCubeMap::TextureCubeMap(const std::string& assetId, int resolution, GLenum internalFormat, int mipLevels)
    : Texture(GL_TEXTURE_CUBE_MAP, internalFormat, resolution, resolution, 1, assetId, AssetType::TextureCubeMap), // resolution is width/height
      mipLevels_(mipLevels)
{
    allocateStorage(mipLevels_);
    setParameters();
}

void TextureCubeMap::allocateStorage(int mipLevels) {
    if (id_ == 0) {
        THROW_GL_EXCEPTION("Texture ID is 0. Cannot allocate storage.");
    }
    glTextureStorage2D(id_, mipLevels, internalFormat_, width_, height_); // DSA: 对 Cubemap ID 分配所有面的存储
    GL_CHECK_ERROR();
}

void TextureCubeMap::setParameters() {
    if (id_ == 0) {
        THROW_GL_EXCEPTION("Texture ID is 0. Cannot set parameters.");
    }
    glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, mipLevels_ > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); // Cubemap 独有
    GL_CHECK_ERROR();
}

void TextureCubeMap::uploadFaceData(int faceIndex, const void* data, GLenum format, GLenum type, int level) {
    if (id_ == 0) {
        THROW_GL_EXCEPTION("Texture ID is 0. Cannot upload face data.");
    }
    int currentResolution = std::max(1, width_ >> level);

    glTextureSubImage3D(id_, level, 0, 0, faceIndex, // xoffset, yoffset, zoffset (face index)
                        currentResolution, currentResolution, 1, // width, height, depth (1 for a single face)
                        format, type, data);
    GL_CHECK_ERROR();
}

// --- 移除静态工厂方法 loadDDS 的实现 ---
// 这些逻辑将移至 TextureManager
/*
std::unique_ptr<TextureCubeMap> TextureCubeMap::loadDDS(const std::string& filePath) { ... }
*/