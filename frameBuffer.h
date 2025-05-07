// Framebuffer.h
#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "GLResource.h"
#include <vector>

class Framebuffer : public GLResource {
public:
    Framebuffer(int width, int height, int numColorAttachments, GLenum colorFormat, bool depthStencil);
    ~Framebuffer() override;

    void bind() const override;
    void unbind() const override;

    GLuint getColorAttachment(int index) const;
    GLuint getDepthStencilAttachment() const;

    void resize(int width, int height);

private:
    std::vector<GLuint> colorAttachments_;
    GLuint depthStencilAttachment_;
    int width_;
    int height_;

    void createColorAttachments(int num, GLenum format);
    void createDepthStencilAttachment(bool useTexture);
    void release() override;
};

#endif // FRAMEBUFFER_H