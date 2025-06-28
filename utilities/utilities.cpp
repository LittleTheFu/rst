#include "utilities.h"
#include <random>
#include <iomanip>

Eigen::Vector3f Utilities::worldToScreenSpace(const Eigen::Vector3f &worldPos,
                                              const Eigen::Matrix4f &viewMatrix,
                                              const Eigen::Matrix4f &projMatrix)
{
    Eigen::Vector4f worldPosHomo(worldPos.x(), worldPos.y(), worldPos.z(), 1.0f);

    // MVP 变换：世界空间 -> 裁剪空间
    Eigen::Vector4f clipSpace = projMatrix * viewMatrix * worldPosHomo;

    // 避免除以零
    if (clipSpace.w() == 0.0f)
        return Eigen::Vector3f(0.0f, 0.0f, 1.0f); // 或 return invalid 值

    // 裁剪空间 -> NDC [-1, 1]
    Eigen::Vector3f ndc = clipSpace.head<3>() / clipSpace.w();

    // NDC -> 屏幕空间 [0, 1]
    float screenX = ndc.x() * 0.5f + 0.5f;
    float screenY = ndc.y() * 0.5f + 0.5f;
    float screenZ = ndc.z(); // NDC 深度

    return Eigen::Vector3f(screenX, screenY, screenZ);
}

bool Utilities::SaveTextureToFile(GLuint textureID,
                                  int width,
                                  int height,
                                  GLenum format,
                                  GLenum type,
                                  const std::string &filename,
                                  bool normalize)
{
    // 正确绑定 2D 纹理
    glBindTextureUnit(0, textureID);

    int channels = 4; // 默认是 RGBA
    if (format == GL_RED || format == GL_DEPTH_COMPONENT)
        channels = 1;
    else if (format == GL_RG)
        channels = 2;
    else if (format == GL_RGB)
        channels = 3;
    else if (format == GL_RGBA)
        channels = 4;
    else
    {
        std::cerr << "Unsupported format passed to SaveTextureToFile.\n";
        return false;
    }

    size_t pixelCount = width * height * channels;

    if (format == GL_DEPTH_COMPONENT && type == GL_FLOAT)
    {
        std::vector<float> floatData(width * height);
        glGetTexImage(GL_TEXTURE_2D, 0, format, type, floatData.data());

        // 找出深度最小值和最大值，用于归一化
        float minDepth = 1.0f, maxDepth = 0.0f;
        for (float v : floatData)
        {
            if (v < minDepth)
                minDepth = v;
            if (v > maxDepth)
                maxDepth = v;
        }

        float range = maxDepth - minDepth;
        if (range < 1e-6f)
            range = 1.0f; // 避免除以零

        std::vector<unsigned char> imageData(width * height);
        for (int i = 0; i < width * height; ++i)
        {
            float val = (floatData[i] - minDepth) / range; // 归一化
            val = std::clamp(val, 0.0f, 1.0f);
            imageData[i] = static_cast<unsigned char>(val * 255.0f);
        }

        // 垂直翻转图像
        for (int j = 0; j < height / 2; ++j)
        {
            for (int i = 0; i < width; ++i)
            {
                std::swap(imageData[j * width + i], imageData[(height - 1 - j) * width + i]);
            }
        }

        return stbi_write_png(filename.c_str(), width, height, 1, imageData.data(), width) != 0;
    }
    else if (type == GL_FLOAT)
    {
        std::vector<float> floatData(pixelCount);
        glGetTexImage(GL_TEXTURE_2D, 0, format, type, floatData.data());

        std::vector<unsigned char> imageData(pixelCount);
        for (size_t i = 0; i < pixelCount; ++i)
        {
            float val = floatData[i];
            if (normalize)
            {
                val = (val * 0.5f) + 0.5f; // 映射 [-1,1] 到 [0,1]
            }
            val = std::clamp(val, 0.0f, 1.0f);
            imageData[i] = static_cast<unsigned char>(val * 255.0f);
        }

        // 垂直翻转图像（OpenGL 原点左下）
        for (int j = 0; j < height / 2; ++j)
        {
            for (int i = 0; i < width * channels; ++i)
            {
                std::swap(imageData[j * width * channels + i],
                          imageData[(height - 1 - j) * width * channels + i]);
            }
        }

        return stbi_write_png(filename.c_str(), width, height, channels, imageData.data(), width * channels) != 0;
    }
    else if (type == GL_UNSIGNED_BYTE)
    {
        std::vector<unsigned char> imageData(pixelCount);
        glGetTexImage(GL_TEXTURE_2D, 0, format, type, imageData.data());

        // 垂直翻转图像
        for (int j = 0; j < height / 2; ++j)
        {
            for (int i = 0; i < width * channels; ++i)
            {
                std::swap(imageData[j * width * channels + i],
                          imageData[(height - 1 - j) * width * channels + i]);
            }
        }

        return stbi_write_png(filename.c_str(), width, height, channels, imageData.data(), width * channels) != 0;
    }

    std::cerr << "Unsupported texture type for saving: only GL_FLOAT and GL_UNSIGNED_BYTE are supported.\n";
    return false;
}

std::string Utilities::generateUniqueTextureId()
{
    static std::random_device rd;
    static std::mt19937_64 gen(rd()); // 使用 64 位生成器
    std::stringstream ss;
    ss << "RuntimeTexture_" << std::hex << std::setfill('0') << std::setw(16) << gen();
    return ss.str();
}