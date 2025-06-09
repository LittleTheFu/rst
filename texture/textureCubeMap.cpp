#include "TextureCubeMap.h"
#include "debug_utils.h"
#include "glException.h" // For THROW_GL_EXCEPTION

// Include gli
#include <gli/gli.hpp>
#include <stdexcept> // For std::runtime_error

// 私有构造函数，供静态工厂方法使用
TextureCubeMap::TextureCubeMap(GLenum target, GLenum internalFormat, int resolution, int mipLevels)
    : Texture(target, internalFormat, resolution, resolution, 1), // depth is 1 for 2D, but for cubemap layers it's implicitly 1 per face
      mipLevels_(mipLevels)
{
    // 不在此处分配存储或设置参数，由静态工厂方法处理
}

// 公有构造函数，用于通用目的（原始数据上传或运行时生成）
TextureCubeMap::TextureCubeMap(int resolution, GLenum internalFormat, int mipLevels)
    : Texture(GL_TEXTURE_CUBE_MAP, internalFormat, resolution, resolution, 1), // resolution is width/height
      mipLevels_(mipLevels)
{
    allocateStorage(mipLevels_);
    setParameters();
}

void TextureCubeMap::allocateStorage(int mipLevels) {
    if (id_ == 0) {
        THROW_GL_EXCEPTION("Texture ID is 0. Cannot allocate storage.");
    }
    // glTextureStorage2D 用于分配所有面的存储
    glTextureStorage2D(id_, mipLevels, internalFormat_, width_, height_); // DSA: 对 Cubemap ID 分配所有面的存储
    GL_CHECK_ERROR();
}

void TextureCubeMap::setParameters() {
    if (id_ == 0) {
        THROW_GL_EXCEPTION("Texture ID is 0. Cannot set parameters.");
    }
    glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, mipLevels_ > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); // Cubemap 独有
    GL_CHECK_ERROR();

    // DDS 纹理通常已经包含预生成的 mipmap，此处通常不需要再次生成。
    // setParameters 的主要作用是设置过滤和缠绕模式。
    // if (mipLevels_ > 1) {
    //     glGenerateTextureMipmap(id_); // No need to call this if DDS already has mipmaps.
    //     GL_CHECK_ERROR();
    // }
}

// 特定方法：上传特定面的数据 (使用 DSA 方式)
// faceIndex: 0-5 对应 GL_TEXTURE_CUBE_MAP_POSITIVE_X 到 GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
void TextureCubeMap::uploadFaceData(int faceIndex, const void* data, GLenum format, GLenum type, int level) {
    if (id_ == 0) {
        THROW_GL_EXCEPTION("Texture ID is 0. Cannot upload face data.");
    }
    // 计算当前 mip level 的分辨率
    int currentResolution = std::max(1, width_ >> level);

    // 对于 DSA，上传 CubeMap 单个层/面数据，使用 glTextureSubImage3D，将 faceIndex 作为 zoffset，深度为 1
    glTextureSubImage3D(id_, level, 0, 0, faceIndex, // xoffset, yoffset, zoffset (face index)
                        currentResolution, currentResolution, 1, // width, height, depth (1 for a single face)
                        format, type, data);
    GL_CHECK_ERROR();
}


// --- Static Factory Method for DDS CubeMap Loading ---
std::unique_ptr<TextureCubeMap> TextureCubeMap::loadDDS(const std::string& filePath) {
    gli::texture gli_texture = gli::load(filePath);

    // 验证纹理是否为空或是否为立方体贴图
    if (gli_texture.empty() || gli_texture.target() != gli::TARGET_CUBE) {
        throw std::runtime_error("Failed to load CubeMap DDS texture from " + filePath + " or it's not a CubeMap texture.");
    }

    // 从 gli 获取纹理属性
    gli::texture::extent_type extent = gli_texture.extent(0); // 获取基准 mip level 的尺寸
    gli::gl GLI_GL_Translator(gli::gl::PROFILE_GL33);
    gli::gl::format GLFormat = GLI_GL_Translator.translate(gli_texture.format(), gli::swizzles());
    GLenum internalFormat = static_cast<GLenum>(GLFormat.Internal);

    int resolution = extent.x; // 立方体贴图是正方形的
    int mipLevels = static_cast<int>(gli_texture.levels());

    // 使用私有构造函数创建 TextureCubeMap 对象
    // 这个构造函数不调用 allocateStorage 或 setParameters
    std::unique_ptr<TextureCubeMap> texture = std::make_unique<TextureCubeMap>(
        GL_TEXTURE_CUBE_MAP, internalFormat, resolution, mipLevels
    );

    // 为纹理分配存储空间
    texture->allocateStorage(mipLevels);

    // 遍历所有面和所有 mipmap 级别并上传数据
    for (int face = 0; face < gli_texture.faces(); ++face) {
        // 根据 gli 的约定，face 0-5 对应 GL_TEXTURE_CUBE_MAP_POSITIVE_X 到 GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
        // 但是 uploadFaceData 需要 0-5 的索引
        // gli 库的 face 顺序是 +X, -X, +Y, -Y, +Z, -Z, 和 OpenGL 的 GL_TEXTURE_CUBE_MAP_POSITIVE_X
        // 枚举的顺序一致，因此可以直接使用 face 索引 0-5。

        for (int level = 0; level < mipLevels; ++level) {
            gli::texture::extent_type level_extent = gli_texture.extent(level);
            // 对于立方体贴图，每个 mip level 的尺寸也是正方形的
            GLsizei level_resolution = level_extent.x;

            // 判断纹理格式是否为压缩格式
            if (gli::is_compressed(gli_texture.format())) {
                // 压缩数据上传
                GLsizei size = static_cast<GLsizei>(gli_texture.size(level));
                // 使用 glCompressedTextureSubImage3D 上传压缩数据到特定面
                glCompressedTextureSubImage3D(texture->id(), level, 0, 0, face, // xoffset, yoffset, zoffset (face index)
                                              level_resolution, level_resolution, 1, // width, height, depth (1 for a single face)
                                              internalFormat, size, gli_texture.data(0, face, level));
                GL_CHECK_ERROR();
            } else {
                // 未压缩数据上传
                GLenum format = gli::gl::external_format(gli_texture.format());
                GLenum type = gli::gl::type_format(gli_texture.format());
                // 使用 uploadFaceData 方法上传未压缩数据
                texture->uploadFaceData(face, gli_texture.data(face, 0, level), format, type, level);
            }
        }
    }

    // 设置纹理参数（过滤、缠绕模式等）
    texture->setParameters();

    return texture;
}