#ifndef UTILITIES_H
#define UTILITIES_H

#include <Eigen/Dense>
#include "stb_image_write.h"
#include <vector>
#include <iostream>
#include <glad/glad.h>

class Utilities
{
public:
    static Eigen::Vector3f worldToScreenSpace(const Eigen::Vector3f &worldPos,
                                              const Eigen::Matrix4f &viewMatrix,
                                              const Eigen::Matrix4f &projMatrix);


    static bool SaveTextureToFile(GLuint textureID,
                                  int width,
                                  int height,
                                  GLenum format,
                                  GLenum type,
                                  const std::string &filename,
                                  bool normalize = false);

    // 注意：这是一个非常简单的ID生成器，生产环境中可能需要更健壮的UUID库
    static std::string generateUniqueTextureId();
};

#endif // UTILITIES_H