#include "Cubemap.h"
#include <glad/glad.h>
#include <iostream>
#include <stb_image.h>

Cubemap::Cubemap() : id_(0) {}

Cubemap::Cubemap(const std::vector<std::string>& faces) : Cubemap() {
    load(faces);
}

Cubemap::~Cubemap() {
    glDeleteTextures(1, &id_);
}

bool Cubemap::load(const std::vector<std::string>& faces) {
    glGenTextures(1, &id_);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id_);

    std::vector<std::string> facePaths = faces;
    if (facePaths.size() != 6) {
        std::cerr << "ERROR::CUBEMAP::Not enough faces provided. Expected 6, got " << facePaths.size() << std::endl;
        return false;
    }

    for (unsigned int i = 0; i < facePaths.size(); ++i) {
        GLenum target;
        switch (i) {
            case 0: target = GL_TEXTURE_CUBE_MAP_POSITIVE_X; break;
            case 1: target = GL_TEXTURE_CUBE_MAP_NEGATIVE_X; break;
            case 2: target = GL_TEXTURE_CUBE_MAP_POSITIVE_Y; break;
            case 3: target = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y; break;
            case 4: target = GL_TEXTURE_CUBE_MAP_POSITIVE_Z; break;
            case 5: target = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; break;
            default:
                std::cerr << "ERROR::CUBEMAP::Invalid face index: " << i << std::endl;
                glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
                glDeleteTextures(1, &id_);
                return false;
        }
        if (!loadFace(facePaths[i], target)) {
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            glDeleteTextures(1, &id_);
            return false;
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return true;
}

void Cubemap::use(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id_);
}

bool Cubemap::loadFace(const std::string& path, GLenum target) {
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(false); // Cubemap 的面通常不需要垂直翻转
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format;
        GLenum internalFormat;
        if (nrChannels == 1) {
            format = GL_RED;
            internalFormat = GL_RED;
        } else if (nrChannels == 3) {
            format = GL_RGB;
            internalFormat = GL_RGB8;
        } else if (nrChannels == 4) {
            format = GL_RGBA;
            internalFormat = GL_RGBA8;
        } else {
            std::cerr << "ERROR::CUBEMAP::Unsupported number of channels: " << nrChannels << " for file: " << path << std::endl;
            stbi_image_free(data);
            return false;
        }

        glTexImage2D(target, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
        return true;
    } else {
        std::cerr << "ERROR::CUBEMAP::Failed to load texture at path: " << path << std::endl;
        stbi_image_free(nullptr);
        return false;
    }
}