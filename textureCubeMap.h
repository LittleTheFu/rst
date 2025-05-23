#ifndef TEXTURE_CUBE_MAP_H
#define TEXTURE_CUBE_MAP_H

#include <string>
#include <glad/glad.h>
// #include <glm/glm.hpp> // 这一行可以删除，因为它不再需要了
#include <gli/gli.hpp> // 这一行必须保留！

class TextureCubeMap
{
public:
    TextureCubeMap();
    ~TextureCubeMap();

    bool loadDDS(const std::string& path);
    void use(unsigned int slot = 0) const;
    GLuint getID() const { return id_; }
    int getSideLength() const { return sideLength_; }
    int getMipLevels() const { return mipLevels_; }

private:
    GLuint id_;
    int sideLength_;
    int mipLevels_;

    TextureCubeMap(const TextureCubeMap&) = delete;
    TextureCubeMap& operator=(const TextureCubeMap&) = delete;
};

#endif // TEXTURE_CUBE_MAP_H