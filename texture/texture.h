#ifndef TEXTURE_H // 保持宏名称一致性
#define TEXTURE_H

#include "glResource.h"
#include "asset.h"     // !!! 新增：包含 IAsset 接口
#include "assetType.h" // !!! 新增：包含 AssetType 枚举
#include <string>      // 用于 std::string

class Texture : public GLResource, public IAsset
{ // !!! 继承 GLResource 和 IAsset
protected:
    GLenum target_; // GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_3D

    // 纹理的通用属性
    GLenum internalFormat_;
    int width_ = 0;
    int height_ = 0;
    int depth_ = 0; // for 3D textures, 1 for 2D/Cubemaps

    std::string m_id; // !!! 新增：IAsset 的唯一ID (通常是文件路径)
    AssetType m_type; // !!! 新增：IAsset 的类型枚举

public:
    // !!! 修改构造函数：现在它接受 assetId 和 assetType 参数
    Texture(GLenum target, GLenum internalFormat, int width, int height, int depth,
            const std::string &assetId, AssetType assetType);

    virtual ~Texture() = default; // 虚析构函数，使用 default 即可

    // --- 实现 IAsset 接口方法 ---
    const std::string &getID() const override { return m_id; }
    AssetType getType() const override { return m_type; }

    // 纹理存储分配 (纯虚函数，子类实现 DSA 版本)
    virtual void allocateStorage(int mipLevels) = 0;

    // 纹理参数设置 (纯虚函数，子类实现 DSA 版本)
    virtual void setParameters() = 0;

    // 激活纹理单元以供着色器采样
    void activate(GLenum textureUnit) const;

    // --- Getter 方法，方便访问内部属性 ---
    GLuint id() const { return id_; } // 从 GLResource 继承
    GLenum target() const { return target_; }
    GLenum internalFormat() const { return internalFormat_; }
    int width() const { return width_; }
    int height() const { return height_; }
    int depth() const { return depth_; }

    void release() override; // 覆盖 GLResource 的纯虚函数
};

#endif // TEXTURE_H