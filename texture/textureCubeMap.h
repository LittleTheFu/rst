#ifndef TEXTURE_CUBE_MAP_H
#define TEXTURE_CUBE_MAP_H

#include "texture.h" // 继承自新的 Texture 基类
#include <memory>
#include <vector>

// Forward declarations for gli if needed
// namespace gli { class texture; }

class TextureCubeMap : public Texture {
private:
    int mipLevels_; // CubeMap 特有的 mipmap 级别

public:
    // !!! 修改私有构造函数：接受 assetId 并传递给基类
    // 供 TextureManager::internalLoadTextureCubeMapDDS 和 internalLoadTextureCubeMapFaces 使用
    TextureCubeMap(const std::string& assetId, GLenum target, GLenum internalFormat, int resolution, int mipLevels);

    // !!! 修改公有构造函数：接受 assetId 并传递给基类
    // 用于运行时生成或直接数据上传的 Cubemap
    TextureCubeMap(const std::string& assetId, int resolution, GLenum internalFormat, int mipLevels = 1);

    virtual ~TextureCubeMap() = default;

    // 实现基类 Texture 的纯虚函数
    void allocateStorage(int mipLevels) override;
    void setParameters() override;

    // 特定方法：上传特定面的数据 (使用 DSA 方式)
    void uploadFaceData(int faceIndex, const void* data, GLenum format, GLenum type, int level = 0);

    // --- 移除静态工厂方法 ---
    // 这些加载逻辑将移至 TextureManager
    // static std::unique_ptr<TextureCubeMap> loadDDS(const std::string& filePath);
};

#endif // TEXTURE_CUBE_MAP_H