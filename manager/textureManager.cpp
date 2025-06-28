#include "textureManager.h"

// For texture loading dependencies
// #define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // For .png, .jpg etc.

#include <gli/gli.hpp> // For .dds

// For error handling and logging
#include "debug_utils.h" // Assuming this has GL_CHECK_ERROR
#include "glException.h" // Assuming this has THROW_GL_EXCEPTION

#include <iostream>
#include <stdexcept> // For std::runtime_error

// Static member initialization (ensures the instance is created only once)
TextureManager& TextureManager::getInstance() {
    static TextureManager instance; // Static local variable ensures it's initialized once
    return instance;
}

// Constructor
TextureManager::TextureManager() {
    // Set stb_image to flip loaded images vertically by default
    // This is often needed because OpenGL expects the first pixel at the bottom-left
    // while most image formats store the first pixel at the top-left.
    stbi_set_flip_vertically_on_load(true);
    std::cout << "TextureManager: Initialized. (stb_image set to flip vertically by default)" << std::endl;
}

// Destructor
TextureManager::~TextureManager() {
    clearAllTextures(); // Ensure all textures are released when the manager is destroyed
    std::cout << "TextureManager: Shut down. All cached textures released." << std::endl;
}

// --- Public Load/Get Interfaces ---

// Updated loadTexture2D to handle both general images and DDS
std::shared_ptr<Texture2D> TextureManager::loadTexture2D(const std::string& path, bool isDDS, bool flipY, bool srgb) {
    // Override stb_image's global flip setting if loading a general image
    if (!isDDS) { // Only set for general images, DDS usually manages its own orientation
        stbi_set_flip_vertically_on_load(flipY);
    }

    if("gold/albedo.png" == path)
    {
        int a = 3;
    }

    // 1. Check cache first
    auto it = m_loaded2DTextures.find(path);
    if (it != m_loaded2DTextures.end()) {
        std::cout << "TextureManager: Reusing cached Texture2D: " << path << std::endl;
        return it->second;
    }

    // 2. Not in cache, try to load internally
    std::shared_ptr<Texture2D> texture = nullptr;
    try {
        if (isDDS) {
            texture = internalLoadTexture2D_DDS(path);
        } else {
            texture = internalLoadTexture2D_Image(path, flipY, srgb);
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "TextureManager ERROR: Failed to load Texture2D from " << path << ": " << e.what() << std::endl;
        return nullptr;
    } catch (const GLException& e) {
        std::cerr << "TextureManager GL ERROR: Failed to load Texture2D from " << path << ": " << e.what() << std::endl;
        return nullptr;
    }


    // 3. If loaded successfully, add to cache
    if (texture) {
        m_loaded2DTextures[path] = texture;
        std::cout << "TextureManager: Loaded and cached Texture2D: " << path << std::endl;
    } else {
        std::cerr << "TextureManager ERROR: internalLoadTexture2D returned nullptr for: " << path << std::endl;
    }
    return texture;
}

std::shared_ptr<TextureCubeMap> TextureManager::loadTextureCubeMap(const std::string& path) {
    // 1. Check cache first
    auto it = m_loadedCubeMaps.find(path);
    if (it != m_loadedCubeMaps.end()) {
        std::cout << "TextureManager: Reusing cached TextureCubeMap (DDS): " << path << std::endl;
        return it->second;
    }

    // 2. Not in cache, try to load internally
    std::shared_ptr<TextureCubeMap> texture = nullptr;
    try {
        texture = internalLoadTextureCubeMapDDS(path);
    } catch (const std::runtime_error& e) {
        std::cerr << "TextureManager ERROR: Failed to load CubeMap DDS from " << path << ": " << e.what() << std::endl;
        return nullptr;
    } catch (const GLException& e) {
        std::cerr << "TextureManager GL ERROR: Failed to load CubeMap DDS from " << path << ": " << e.what() << std::endl;
        return nullptr;
    }

    // 3. If loaded successfully, add to cache
    if (texture) {
        m_loadedCubeMaps[path] = texture;
        std::cout << "TextureManager: Loaded and cached TextureCubeMap (DDS): " << path << std::endl;
    } else {
        std::cerr << "TextureManager ERROR: internalLoadTextureCubeMapDDS returned nullptr for: " << path << std::endl;
    }
    return texture;
}

std::shared_ptr<TextureCubeMap> TextureManager::loadTextureCubeMap(const std::vector<std::string>& faces) {
    // Generate a unique ID for multi-face Cubemap from the concatenated paths
    std::string compositeId;
    for (const auto& face : faces) {
        compositeId += face + "|";
    }

    // 1. Check cache first
    auto it = m_loadedCubeMaps.find(compositeId);
    if (it != m_loadedCubeMaps.end()) {
        std::cout << "TextureManager: Reusing cached TextureCubeMap (Faces): " << compositeId << std::endl;
        return it->second;
    }

    // 2. Not in cache, try to load internally
    std::shared_ptr<TextureCubeMap> texture = nullptr;
    try {
        texture = internalLoadTextureCubeMapFaces(faces);
    } catch (const std::runtime_error& e) {
        std::cerr << "TextureManager ERROR: Failed to load CubeMap from faces (" << compositeId << "): " << e.what() << std::endl;
        return nullptr;
    } catch (const GLException& e) {
        std::cerr << "TextureManager GL ERROR: Failed to load CubeMap from faces (" << compositeId << "): " << e.what() << std::endl;
        return nullptr;
    }

    // 3. If loaded successfully, add to cache
    if (texture) {
        m_loadedCubeMaps[compositeId] = texture;
        std::cout << "TextureManager: Loaded and cached TextureCubeMap (Faces) with ID: " << compositeId << std::endl;
    } else {
        std::cerr << "TextureManager ERROR: internalLoadTextureCubeMapFaces returned nullptr for ID: " << compositeId << std::endl;
    }
    return texture;
}


std::shared_ptr<Texture2D> TextureManager::getTexture2D(const std::string& path) {
    auto it = m_loaded2DTextures.find(path);
    if (it != m_loaded2DTextures.end()) {
        return it->second;
    }
    return nullptr; // Not found
}

std::shared_ptr<TextureCubeMap> TextureManager::getTextureCubeMap(const std::string& path) {
    auto it = m_loadedCubeMaps.find(path);
    if (it != m_loadedCubeMaps.end()) {
        return it->second;
    }
    return nullptr; // Not found
}

void TextureManager::unloadTexture(const std::string& path) {
    // Attempt to remove from 2D texture cache
    if (m_loaded2DTextures.erase(path) > 0) {
        std::cout << "TextureManager: Unloaded Texture2D: " << path << std::endl;
    }
    // Attempt to remove from Cubemap cache
    else if (m_loadedCubeMaps.erase(path) > 0) {
        std::cout << "TextureManager: Unloaded TextureCubeMap: " << path << std::endl;
    } else {
        std::cout << "TextureManager: Texture not found in cache for unload: " << path << std::endl;
    }
}

void TextureManager::clearAllTextures() {
    m_loaded2DTextures.clear();   // Clears all 2D textures
    m_loadedCubeMaps.clear();     // Clears all Cubemap textures
    std::cout << "TextureManager: All textures cleared from cache." << std::endl;
}

// --- Internal Loading Helper Functions ---

// Handles general 2D image formats (PNG, JPG, etc.) using stb_image
std::shared_ptr<Texture2D> TextureManager::internalLoadTexture2D_Image(const std::string& path, bool flipY, bool srgb) {
    // 设置 stb_image 的垂直翻转
    stbi_set_flip_vertically_on_load(flipY);

    int width, height, numChannels;
    // **强制 stb_image 将图片加载为 4 个通道 (RGBA)。**
    // 即使原图是 RGB 或灰度图，也会被转换为 RGBA。
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &numChannels, 4); 

    if (!data) {
        throw std::runtime_error("Failed to load image from " + path + ": " + stbi_failure_reason());
    }

    // 由于我们强制加载为 4 通道，internalFormat 和 format 都将使用 RGBA 相关格式。
    // 如果 sRGB 被请求，则使用 sRGB 格式，否则使用标准 RGBA8。
    GLenum internalFormat = srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;
    GLenum format = GL_RGBA; // 数据上传格式固定为 GL_RGBA

    // 计算 mipmap 级别数量
    int mipLevels = 1;
    // 只有当纹理尺寸大于 1 且有有效通道时才考虑生成 mipmap。
    // (numChannels 理论上现在固定为 4，但保留这个检查以防万一)
    // if (std::max(width, height) > 1 && numChannels > 0) {
    //     mipLevels = static_cast<int>(std::floor(std::log2(std::max(width, height)))) + 1;
    // }

    // 创建 Texture2D 对象
    std::shared_ptr<Texture2D> texture = std::make_shared<Texture2D>(
        path, GL_TEXTURE_2D, internalFormat, width, height, mipLevels);
    GL_CHECK_ERROR();

     // --- !!! 核心修复点 !!! ---
    // 必须在上传数据之前为纹理分配 GPU 内存
    texture->allocateStorage(mipLevels);
    GL_CHECK_ERROR(); // 检查 allocateStorage 自身是否成功
    // --- !!! 核心修复点结束 !!! ---

    // **上传数据时，使用固定的 GL_RGBA 格式。**
    // 这与 stbi_load 强制加载的 4 通道数据相匹配。
    texture->uploadData(data, format, GL_UNSIGNED_BYTE, 0);

    // 如果需要，生成 mipmap
    if (mipLevels > 1) {
        glGenerateTextureMipmap(texture->id());
        GL_CHECK_ERROR();
    }

    // 设置纹理参数（过滤、缠绕模式等）
    texture->setParameters();

    // 释放 stb_image 分配的内存
    stbi_image_free(data);

    return texture;
}

// Handles 2D DDS texture loading using gli
std::shared_ptr<Texture2D> TextureManager::internalLoadTexture2D_DDS(const std::string& path) {
    gli::texture gli_texture = gli::load(path);

    if (gli_texture.empty() || gli_texture.target() != gli::TARGET_2D) {
        throw std::runtime_error("Failed to load 2D DDS texture from " + path + " or it's not a 2D texture.");
    }

    gli::texture::extent_type extent = gli_texture.extent(0);
    gli::gl GLI_GL_Translator(gli::gl::PROFILE_GL33);
    gli::gl::format GLFormat = GLI_GL_Translator.translate(gli_texture.format(), gli::swizzles());
    GLenum internalFormat = static_cast<GLenum>(GLFormat.Internal);

    int width = extent.x;
    int height = extent.y;
    int mipLevels = static_cast<int>(gli_texture.levels());

    std::shared_ptr<Texture2D> texture = std::make_shared<Texture2D>(
        path, GL_TEXTURE_2D, internalFormat, width, height, mipLevels);

    texture->allocateStorage(mipLevels);

    for (int level = 0; level < mipLevels; ++level) {
        gli::texture::extent_type level_extent = gli_texture.extent(level);
        GLsizei level_width = level_extent.x;
        GLsizei level_height = level_extent.y;

        if (gli::is_compressed(gli_texture.format())) {
            GLsizei size = static_cast<GLsizei>(gli_texture.size(level));
            glCompressedTextureSubImage2D(texture->id(), level, 0, 0, level_width, level_height, internalFormat, size, gli_texture.data(0, 0, level));
            GL_CHECK_ERROR();
        } else {
            GLenum format = gli::gl::external_format(gli_texture.format());
            GLenum type = gli::gl::type_format(gli_texture.format());
            texture->uploadData(gli_texture.data(0, 0, level), format, type, level);
        }
    }

    texture->setParameters();

    return texture;
}

std::shared_ptr<TextureCubeMap> TextureManager::internalLoadTextureCubeMapDDS(const std::string& path) {
    gli::texture gli_texture = gli::load(path);

    if (gli_texture.empty() || gli_texture.target() != gli::TARGET_CUBE) {
        throw std::runtime_error("Failed to load CubeMap DDS texture from " + path + " or it's not a CubeMap texture.");
    }

    gli::texture::extent_type extent = gli_texture.extent(0);
    gli::gl GLI_GL_Translator(gli::gl::PROFILE_GL33);
    gli::gl::format GLFormat = GLI_GL_Translator.translate(gli_texture.format(), gli::swizzles());
    GLenum internalFormat = static_cast<GLenum>(GLFormat.Internal);

    int resolution = extent.x; // Cubemap resolution (width == height)
    int mipLevels = static_cast<int>(gli_texture.levels());

    std::shared_ptr<TextureCubeMap> texture = std::make_shared<TextureCubeMap>(
        path, GL_TEXTURE_CUBE_MAP, internalFormat, resolution, mipLevels);

    texture->allocateStorage(mipLevels);

    for (int face = 0; face < gli_texture.faces(); ++face) {
        for (int level = 0; level < mipLevels; ++level) {
            gli::texture::extent_type level_extent = gli_texture.extent(level);
            GLsizei level_resolution = level_extent.x;

            if (gli::is_compressed(gli_texture.format())) {
                GLsizei size = static_cast<GLsizei>(gli_texture.size(level));
                glCompressedTextureSubImage3D(texture->id(), level, 0, 0, face,
                                                level_resolution, level_resolution, 1,
                                                internalFormat, size, gli_texture.data(0, face, level));
                GL_CHECK_ERROR();
            } else {
                GLenum format = gli::gl::external_format(gli_texture.format());
                GLenum type = gli::gl::type_format(gli_texture.format());
                texture->uploadFaceData(face, gli_texture.data(face, 0, level), format, type, level);
            }
        }
    }

    texture->setParameters();

    return texture;
}

std::shared_ptr<TextureCubeMap> TextureManager::internalLoadTextureCubeMapFaces(const std::vector<std::string>& faces) {
    if (faces.size() != 6) {
        throw std::runtime_error("CubeMap from faces requires exactly 6 image paths.");
    }

    // Load the first face to determine common properties
    int width, height, numChannels;
    stbi_set_flip_vertically_on_load(false); // Cubemap faces usually don't need vertical flipping
    unsigned char* data = stbi_load(faces[0].c_str(), &width, &height, &numChannels, 4); // Force 4 channels (RGBA)
    stbi_set_flip_vertically_on_load(true); // Reset for other texture loads

    if (!data) {
        throw std::runtime_error("Failed to load first CubeMap face from " + faces[0] + ": " + stbi_failure_reason());
    }
    if (width != height) {
        stbi_image_free(data);
        throw std::runtime_error("CubeMap faces must be square. Face " + faces[0] + " is " + std::to_string(width) + "x" + std::to_string(height));
    }

    GLenum internalFormat = GL_RGBA8; // Assuming standard RGBA for now for image files
    // If you need sRGB for cubemaps from images, you might need a parameter here.
    // internalFormat = srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;

    // Calculate mipLevels for images (can be turned off with a parameter if needed)
    int mipLevels = 1;
    // if (std::max(width, height) > 1 && numChannels > 0) {
    //     mipLevels = static_cast<int>(std::floor(std::log2(std::max(width, height)))) + 1;
    // }

    // Generate a unique ID for this set of faces
    std::string compositeId;
    for (const auto& facePath : faces) {
        compositeId += facePath + "|";
    }

    std::shared_ptr<TextureCubeMap> texture = std::make_shared<TextureCubeMap>(
        compositeId, GL_TEXTURE_CUBE_MAP, internalFormat, width, mipLevels);

    texture->allocateStorage(mipLevels);

    texture->uploadFaceData(0, data, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    stbi_image_free(data);

    for (size_t i = 1; i < 6; ++i) {
        stbi_set_flip_vertically_on_load(false); // No flipping for cubemap faces
        unsigned char* faceData = stbi_load(faces[i].c_str(), &width, &height, &numChannels, 4);
        stbi_set_flip_vertically_on_load(true); // Reset

        if (!faceData) {
            throw std::runtime_error("Failed to load CubeMap face " + std::to_string(i) + " from " + faces[i] + ": " + stbi_failure_reason());
        }
        if (width != texture->width() || height != texture->height()) {
            stbi_image_free(faceData);
            throw std::runtime_error("CubeMap faces must have consistent dimensions. Face " + faces[i] + " is " + std::to_string(width) + "x" + std::to_string(height) + ", expected " + std::to_string(texture->width()) + "x" + std::to_string(texture->height()));
        }

        texture->uploadFaceData(static_cast<int>(i), faceData, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        stbi_image_free(faceData);
    }

    if (mipLevels > 1) {
        glGenerateTextureMipmap(texture->id());
        GL_CHECK_ERROR();
    }

    texture->setParameters();

    return texture;
}