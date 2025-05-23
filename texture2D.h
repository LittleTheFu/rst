#ifndef TEXTURE_2D_H
#define TEXTURE_2D_H

#include <string>
#include <glad/glad.h>
#include <gli/gli.hpp> // 包含 gli 库

class Texture2D
{
public:
    Texture2D();
    ~Texture2D();

    // 加载 DDS 格式的 2D 纹理 (例如 BRDF LUT)
    // gli::load 会自动处理 HDR (浮点) 数据和 mipmap
    bool loadDDS(const std::string& path);

    // 绑定纹理到指定的纹理单元
    void use(unsigned int slot = 0) const;

    // 获取 OpenGL 纹理 ID
    GLuint getID() const { return id_; }

    // 获取纹理宽度
    int getWidth() const { return width_; }

    // 获取纹理高度
    int getHeight() const { return height_; }

    // 获取 mipmap 级别数量
    int getMipLevels() const { return mipLevels_; }

private:
    GLuint id_;           // OpenGL 纹理 ID
    int width_;           // 纹理宽度 (mip level 0)
    int height_;          // 纹理高度 (mip level 0)
    int mipLevels_;       // 包含的 mipmap 级别数量

    // 禁用拷贝构造函数和赋值运算符，防止不安全的拷贝
    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;
};

#endif // TEXTURE_2D_H