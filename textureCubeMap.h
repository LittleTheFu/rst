#ifndef _RENDER_TEXTURE_CUBE_MAP_H_
#define _RENDER_TEXTURE_CUBE_MAP_H_

#include "Texture.h"
#include <string> // For std::string
#include <memory> // For std::unique_ptr
#include <gli/gli.hpp> // For gli::load_dds_cube

class TextureCubeMap : public Texture {
// private:
public:
    int mipLevels_;

    // 私有构造函数，供静态工厂方法（如 loadDDS）使用
    // 不会立即分配存储或设置参数，这些由 loadDDS 根据 DDS 数据处理
    TextureCubeMap(GLenum target, GLenum internalFormat, int resolution, int mipLevels);

public:
    // 公有构造函数，用于通用目的（原始数据上传或运行时生成）
    // 这个会立即分配存储并设置参数。
    TextureCubeMap(int resolution, GLenum internalFormat, int mipLevels = 1);

    // 静态工厂方法：使用 gli 加载 DDS 立方体贴图
    static std::unique_ptr<TextureCubeMap> loadDDS(const std::string& filePath);

    // 重写基类的纯虚函数
    void allocateStorage(int mipLevels) override;
    void setParameters() override;

    // 特定方法：上传特定面的数据 (使用 DSA 方式)
    // faceIndex: 0-5 对应 GL_TEXTURE_CUBE_MAP_POSITIVE_X 到 GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    void uploadFaceData(int faceIndex, const void* data, GLenum format, GLenum type, int level = 0);

    // 获取分辨率 (宽高等于分辨率)
    int getResolution() const { return width_; } // resolution_ 在基类中可以用 width_ 表示
    int getMipLevels() const { return mipLevels_; } // Added getter for mip levels
};

#endif // _RENDER_TEXTURE_CUBE_MAP_H_