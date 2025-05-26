#ifndef _RENDER_TEXTURE_2D_H_
#define _RENDER_TEXTURE_2D_H_

#include "Texture.h"

class Texture2D : public Texture {
private:
    int mipLevels_;

public:
    // 构造函数，初始化并分配存储
    Texture2D(int width, int height, GLenum internalFormat, int mipLevels = 1);

    // 重写基类的纯虚函数
    void allocateStorage(int mipLevels) override;
    void setParameters() override;

    // 特定方法：上传数据 (仍然需要 format 和 type)
    void uploadData(const void* data, GLenum format, GLenum type, int level = 0);

    // 获取尺寸
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
};

#endif