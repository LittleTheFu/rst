#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "glResource.h"
#include <glad/glad.h>
#include <vector>
#include <stdexcept>


class Framebuffer : public GLResource {
    
private:
    int width_;
    int height_;

public:
    Framebuffer(int width, int height);
    ~Framebuffer() override = default;

    void activate() const;
    void deactivate() const;

    // --- 附件管理方法 (通过传入纹理/Renderbuffer ID) ---

    // 附加颜色纹理
    void attachColorTexture(GLuint textureID, GLenum attachmentPoint, int mipLevel = 0);

    // 附加深度纹理 (通常用于 2D 纹理)
    void attachDepthTexture(GLuint textureID, int mipLevel = 0);

    // 附加深度/模板纹理
    void attachDepthStencilTexture(GLuint textureID, int mipLevel = 0);

    // 附加深度 Renderbuffer
    void attachDepthRenderbuffer(GLuint renderbufferID);

    // 附加深度/模板 Renderbuffer
    void attachDepthStencilRenderbuffer(GLuint renderbufferID);

    // --- 新增方法：附加立方体贴图的特定面作为深度附件 ---
    // textureID: 立方体贴图的 ID
    // cubeMapFaceTarget: GL_TEXTURE_CUBE_MAP_POSITIVE_X + i (具体的面)
    // mipLevel: Mipmap 级别
    void attachDepthCubeMapFace(GLuint textureID, GLenum cubeMapFaceTarget, int mipLevel = 0);


    void setDrawBuffers(const std::vector<GLenum>& drawBuffers);
    void checkCompleteness() const;

    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

protected:
    void release() override;
};

#endif // FRAMEBUFFER_H