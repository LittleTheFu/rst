#pragma once

#include "asset.h"         // 引入 IAsset 接口
#include "texture2D.h"      // 引入 Texture2D 类
#include "textureCubeMap.h" // 引入 TextureCubeMap 类

#include <string>           // 用于 std::string
#include <map>              // 用于 std::map 作为缓存容器
#include <memory>           // 用于 std::shared_ptr
#include <vector>           // 用于 TextureCubeMap 的多文件加载

#include <iostream> // For basic logging

/**
 * @brief TextureManager 是一个单例类，负责纹理资源的加载、缓存和管理。
 *
 * 它确保每个纹理文件在内存中只加载一次，并能被应用程序中的多个部分共享。
 */
class TextureManager {
public:
    /**
     * @brief 获取 TextureManager 的唯一实例。
     * @return TextureManager 实例的引用。
     */
    static TextureManager& getInstance();

    // --- 公共加载/获取接口 ---

    /**
     * @brief 加载或获取一个 2D 纹理。
     *
     * 如果纹理已在缓存中，则直接返回其共享指针；否则，从文件加载并缓存。
     *
     * @param path 纹理文件的完整路径。
     * @param isDDS 是否是 DDS 文件。如果是 true，则使用 DDS 加载器；否则使用通用图像加载器 (stb_image)。
     * @param flipY 如果是通用图像文件，是否垂直翻转纹理。
     * @param srgb 如果是通用图像文件，是否将纹理解释为 sRGB 颜色空间。
     * @return 纹理的 std::shared_ptr，如果加载失败则返回 nullptr。
     */
    std::shared_ptr<Texture2D> loadTexture2D(const std::string& path, bool isDDS = false, bool flipY = true, bool srgb = false); // flipY 默认 true 更符合常用情况

    /**
     * @brief 加载或获取一个立方体贴图纹理（DDS 格式）。
     *
     * 如果纹理已在缓存中，则直接返回其共享指针；否则，从 DDS 文件加载并缓存。
     *
     * @param path DDS 文件的完整路径。
     * @return 纹理的 std::shared_ptr，如果加载失败则返回 nullptr。
     */
    std::shared_ptr<TextureCubeMap> loadTextureCubeMap(const std::string& path);

    /**
     * @brief 加载或获取一个立方体贴图纹理（多张图片）。
     *
     * 如果纹理已在缓存中，则根据唯一的组合路径字符串返回其共享指针；
     * 否则，从六个面文件加载并缓存。
     *
     * @param faces 包含六个面图片路径的 std::vector。顺序通常为：右、左、上、下、前、后。
     * @return 纹理的 std::shared_ptr，如果加载失败则返回 nullptr。
     */
    std::shared_ptr<TextureCubeMap> loadTextureCubeMap(const std::vector<std::string>& faces);

    /**
     * @brief 尝试从缓存中获取一个已加载的 2D 纹理。
     * @param path 纹理的 ID (文件路径)。
     * @return 纹理的 std::shared_ptr，如果未找到则返回 nullptr。
     */
    std::shared_ptr<Texture2D> getTexture2D(const std::string& path);

    /**
     * @brief 尝试从缓存中获取一个已加载的立方体贴图纹理。
     * @param path 纹理的 ID (文件路径)。
     * @return 纹理的 std::shared_ptr，如果未找到则返回 nullptr。
     */
    std::shared_ptr<TextureCubeMap> getTextureCubeMap(const std::string& path);

    /**
     * @brief 卸载指定路径的纹理。
     *
     * 这将从缓存中移除纹理。如果它是最后一个引用，
     * 纹理对象本身（及其 OpenGL 资源）将被释放。
     *
     * @param path 要卸载纹理的 ID。
     */
    void unloadTexture(const std::string& path);

    /**
     * @brief 清空所有已缓存的纹理。
     *
     * 强制释放所有缓存的纹理，即使它们可能仍被其他 shared_ptr 引用。
     * 这应谨慎使用，通常在程序结束或大规模场景切换时。
     */
    void clearAllTextures();

private:
    // --- 单例模式的私有成员 ---
    TextureManager();                                     // 私有构造函数
    ~TextureManager();                                    // 私有析构函数
    TextureManager(const TextureManager&) = delete;             // 禁用拷贝构造函数
    TextureManager& operator=(const TextureManager&) = delete;  // 禁用赋值运算符

    // --- 内部纹理缓存 ---
    std::map<std::string, std::shared_ptr<Texture2D>> m_loaded2DTextures;
    std::map<std::string, std::shared_ptr<TextureCubeMap>> m_loadedCubeMaps;

    // --- 内部加载辅助函数 ---
    // 加载通用 2D 图像 (PNG, JPG, etc.)
    std::shared_ptr<Texture2D> internalLoadTexture2D_Image(const std::string& path, bool flipY, bool srgb);
    // 加载 2D DDS 纹理
    std::shared_ptr<Texture2D> internalLoadTexture2D_DDS(const std::string& path);
    // 加载 Cubemap DDS 纹理
    std::shared_ptr<TextureCubeMap> internalLoadTextureCubeMapDDS(const std::string& path);
    // 加载 Cubemap 从 6 个面图像
    std::shared_ptr<TextureCubeMap> internalLoadTextureCubeMapFaces(const std::vector<std::string>& faces);
};