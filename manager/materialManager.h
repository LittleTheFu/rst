#ifndef MATERIAL_MANAGER_H
#define MATERIAL_MANAGER_H

#include "asset.h"        // 引入 IAsset 接口 (因为 Material 继承 IAsset)
#include "material.h"     // 引入 Material 类
#include "textureManager.h" // MaterialManager 会依赖 TextureManager 来加载纹理
#include "MaterialFactory.h" // MaterialManager 会依赖 MaterialFactory 来创建材质

#include <string>
#include <map>
#include <memory>
#include <iostream> // For basic logging

/**
 * @brief MaterialManager 是一个单例类，负责材质资源的加载、缓存和管理。
 *
 * 它确保每个材质在内存中只加载一次，并能被应用程序中的多个部分共享。
 * MaterialManager 依赖于 MaterialFactory 来执行实际的材质创建和纹理加载。
 */
class MaterialManager {
public:
    /**
     * @brief 获取 MaterialManager 的唯一实例。
     * @return MaterialManager 实例的引用。
     */
    static MaterialManager& getInstance();

    // --- 公共加载/获取接口 ---

    /**
     * @brief 加载或获取一个材质。
     *
     * 如果材质已在缓存中，则直接返回其共享指针；否则，通过 MaterialFactory 加载并缓存。
     * 此函数封装了通过目录加载材质的常用模式。
     *
     * @param materialID 材质的唯一标识符（通常是其名称或一个指向材质文件的路径）。
     * @param directory 可选：如果材质需要从文件加载，指定包含材质纹理的目录。
     * 如果 materialID 已经足以唯一标识一个预设材质或内部生成材质，
     * 则此参数可为空。
     * @param extension 可选：纹理文件扩展名，默认为 ".png"。
     * @param flipY 可选：是否垂直翻转加载的纹理，默认为 true。
     * @return 材质的 std::shared_ptr，如果加载失败则返回 nullptr。
     */
    std::shared_ptr<Material> loadMaterial(
        const std::string& materialID,
        const std::string& directory = "",
        const std::string& extension = ".png",
        bool flipY = true
    );

    /**
     * @brief 尝试从缓存中获取一个已加载的材质。
     * @param materialID 材质的唯一标识符。
     * @return 材质的 std::shared_ptr，如果未找到则返回 nullptr。
     */
    std::shared_ptr<Material> getMaterial(const std::string& materialID);

    /**
     * @brief 卸载指定 ID 的材质。
     *
     * 这将从缓存中移除材质。如果它是最后一个引用，
     * 材质对象本身（及其内部持有的纹理 shared_ptr）将被释放。
     *
     * @param materialID 要卸载材质的 ID。
     */
    void unloadMaterial(const std::string& materialID);

    /**
     * @brief 清空所有已缓存的材质。
     *
     * 强制释放所有缓存的材质。MaterialManager 自身不持有资源的唯一所有权，
     * 但清空 map 会减少 shared_ptr 的引用计数。如果外部没有其他引用，资源将被释放。
     * 这应谨慎使用，通常在程序结束或大规模场景切换时。
     */
    void clearAllMaterials();

private:
    // --- 单例模式的私有成员 ---
    MaterialManager();                                     // 私有构造函数
    ~MaterialManager();                                    // 私有析构函数
    MaterialManager(const MaterialManager&) = delete;             // 禁用拷贝构造函数
    MaterialManager& operator=(const MaterialManager&) = delete;  // 禁用赋值运算符

    // --- 内部材质缓存 ---
    // 键是材质的唯一 ID (string)，值是材质的共享指针
    std::map<std::string, std::shared_ptr<Material>> m_loadedMaterials;

    // --- 内部加载辅助函数 ---
    // 这个函数封装了调用 MaterialFactory 来创建材质的逻辑，并处理可能的异常
    std::shared_ptr<Material> internalLoadMaterial(
        const std::string& materialID,
        const std::string& directory,
        const std::string& extension,
        bool flipY
    );
};

#endif // MATERIAL_MANAGER_H
