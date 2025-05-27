#ifndef _RENDER_TEXTURE_2D_H_
#define _RENDER_TEXTURE_2D_H_

#include "Texture.h"
#include <string> // For std::string
#include <memory> // For std::unique_ptr
#include <gli/gli.hpp> // For gli::load_dds function

class Texture2D : public Texture {
// private:
public:
    int mipLevels_; // Store the actual number of mip levels allocated/used

    // Private constructor for use by static factory methods (like loadDDS)
    // This allows creating an object without immediately calling allocateStorage/setParameters
    // as loadDDS will handle allocation and parameters based on DDS data.
    Texture2D(GLenum target, GLenum internalFormat, int width, int height, int mipLevels);

public:
    // Public constructor for general purpose (raw data upload or generation)
    // This one allocates storage and sets parameters immediately.
    Texture2D(int width, int height, GLenum internalFormat, int mipLevels = 1);

    // Static factory method to load a 2D DDS texture using gli
    static std::unique_ptr<Texture2D> loadDDS(const std::string& filePath);

    // 重写基类的纯虚函数
    void allocateStorage(int mipLevels) override;
    void setParameters() override; // This method can be generalized or specific to raw data textures

    // 特定方法：上传数据 (仍然需要 format 和 type)
    void uploadData(const void* data, GLenum format, GLenum type, int level = 0);

    // 获取尺寸
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    int getMipLevels() const { return mipLevels_; } // Added getter for mip levels
};

#endif