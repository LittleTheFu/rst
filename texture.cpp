#include "Texture.h"
#include <glad/glad.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h> // 或者你选择的图像加载库

Texture::Texture() : id_(0), width_(0), height_(0), nrChannels_(0), format_(0), internalFormat_(0), data_(nullptr) {}

Texture::Texture(const std::string& path) : Texture() {
    load(path);
}

Texture::~Texture() {
    glDeleteTextures(1, &id_);
    stbi_image_free(data_);
}

bool Texture::load(const std::string& path) {
    if (!loadTextureFromFile(path)) {
        std::cerr << "ERROR::TEXTURE::Failed to load texture: " << path << std::endl;
        return false;
    }
    return true;
}

void Texture::use(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, id_);
}

bool Texture::loadTextureFromFile(const std::string& path) {
    stbi_set_flip_vertically_on_load(true); // 翻转y轴
    data_ = stbi_load(path.c_str(), &width_, &height_, &nrChannels_, 0);
    if (data_) {
        if (nrChannels_ == 1) {
            format_ = GL_RED;
            internalFormat_ = GL_RED;
        } else if (nrChannels_ == 3) {
            format_ = GL_RGB;
            internalFormat_ = GL_RGB8; // 或 GL_SRGB
        } else if (nrChannels_ == 4) {
            format_ = GL_RGBA;
            internalFormat_ = GL_RGBA8; // 或 GL_SRGB_ALPHA
        }

        glGenTextures(1, &id_);
        glBindTexture(GL_TEXTURE_2D, id_);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat_, width_, height_, 0, format_, GL_UNSIGNED_BYTE, data_);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // stbi_image_free(data_);
        return true;
    } else {
        std::cerr << "ERROR::TEXTURE::Texture failed to load at path: " << path << std::endl;
        // stbi_image_free(data_);
        return false;
    }
}