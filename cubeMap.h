#ifndef CUBEMAP_H
#define CUBEMAP_H

#include <string>
#include <vector>
#include <glad/glad.h>

class Cubemap {
public:
    Cubemap();
    Cubemap(const std::vector<std::string>& faces);
    ~Cubemap();

    bool load(const std::vector<std::string>& faces);
    void use(unsigned int slot = 0) const; // 激活并绑定 cubemap
    GLuint getID() const { return id_; }

private:
    GLuint id_;

    bool loadFace(const std::string& path, GLenum target);
};

#endif