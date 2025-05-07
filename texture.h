#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <glad/glad.h>

class Texture {
public:
    Texture();
    Texture(const std::string& path);
    ~Texture();

    bool load(const std::string& path);
    void use(unsigned int slot = 0) const; // 激活并绑定纹理
    GLuint getID() const { return id_; }
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

private:
    GLuint id_;
    int width_;
    int height_;
    int nrChannels_;
    GLenum format_;
    GLenum internalFormat_;
    unsigned char* data_;

    bool loadTextureFromFile(const std::string& path);
};

#endif