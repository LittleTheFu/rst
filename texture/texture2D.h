#ifndef TEXTURE2D_H
#define TEXTURE2D_H

#include "texture.h" // Texture2D 现在继承自新的 Texture 基类
#include <memory> 
#include <vector> // 如果 internalLoadTexture2D 需要它


class Texture2D : public Texture {
private:
    int mipLevels_; // Texture2D 特有的 mipmap 级别

public:
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

    // Getter for mipLevels_
    int getMipLevels() const { return mipLevels_; }
};

#endif // TEXTURE2D_H