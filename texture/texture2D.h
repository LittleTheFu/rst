#ifndef TEXTURE2D_H
#define TEXTURE2D_H

#include "texture.h" // Texture2D 现在继承自新的 Texture 基类
#include <memory> 
#include <vector> // 如果 internalLoadTexture2D 需要它

// Forward declarations for stb_image and gli related functions if needed
// namespace gli { class texture; }

class Texture2D : public Texture {
private:
    int mipLevels_; // Texture2D 特有的 mipmap 级别

public:
    // !!! 修改构造函数：现在接受 assetId，并将其传递给基类
    // 供 TextureManager::internalLoadTexture2D 使用
    Texture2D(const std::string& assetId, GLenum target, GLenum internalFormat, int width, int height, int mipLevels);
    
    // !!! 修改公有构造函数：用于运行时生成纹理，同样需要 assetId
    Texture2D(const std::string& assetId, int width, int height, GLenum internalFormat, int mipLevels = 1);

    // 析构函数（默认即可，基类会处理 OpenGL ID 释放）
    virtual ~Texture2D() = default;

    // 实现基类 Texture 的纯虚函数
    void allocateStorage(int mipLevels) override;
    void setParameters() override;

    // 上传数据到特定 mip 级别 (DSA 方式)
    void uploadData(const void *data, GLenum format, GLenum type, int level = 0);

    // --- 移除静态工厂方法 ---
    // 这些加载逻辑将移至 TextureManager
    // static std::unique_ptr<Texture2D> loadDDS(const std::string &filePath);
    // static std::unique_ptr<Texture2D> loadFromFile(const std::string &filePath, bool generateMipmaps = false, bool useSRGB = false);

    // Getter for mipLevels_
    int getMipLevels() const { return mipLevels_; }
};

#endif // TEXTURE2D_H