#ifndef _RENDER_TEXTURE_H_
#define _RENDER_TEXTURE_H_

#include "GLResource.h"

class Texture : public GLResource {
protected:
    GLenum target_; // GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_3D

    // 纹理的通用属性
    GLenum internalFormat_;
    int width_ = 0;
    int height_ = 0;
    int depth_ = 0; // for 3D textures, resolution for cubemaps

public:
    Texture(GLenum target, GLenum internalFormat, int width, int height, int depth = 1);
    virtual ~Texture() = default;

    // 纹理存储分配 (纯虚函数，子类实现 DSA 版本)
    virtual void allocateStorage(int mipLevels) = 0;

    // 纹理参数设置 (纯虚函数，子类实现 DSA 版本)
    virtual void setParameters() = 0;

    // 激活纹理单元以供着色器采样 (这个仍然需要，因为着色器通过纹理单元访问)
    void activate(GLenum textureUnit) const;


protected:
    // GLObject 的资源释放
    void release() override;
};

#endif // TEXTURE_H