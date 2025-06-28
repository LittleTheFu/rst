#include "MaterialManager.h"
#include <stdexcept> // For std::runtime_error and std::exception

// Static member initialization: 确保 MaterialManager 实例只在第一次访问时创建
MaterialManager& MaterialManager::getInstance() {
    static MaterialManager instance;
    return instance;
}

// 构造函数：初始化管理器
MaterialManager::MaterialManager() {
    std::cout << "MaterialManager: Initialized." << std::endl;
}

// 析构函数：在管理器销毁时清除所有缓存的材质
MaterialManager::~MaterialManager() {
    clearAllMaterials();
    std::cout << "MaterialManager: Shut down. All cached materials released." << std::endl;
}

// --- 公共加载/获取接口实现 ---

// 加载或获取一个材质
std::shared_ptr<Material> MaterialManager::loadMaterial(
    const std::string& materialID,
    const std::string& directory,
    const std::string& extension,
    bool flipY)
{
    // 1. 检查缓存：如果材质已存在，则直接返回
    auto it = m_loadedMaterials.find(materialID);
    if (it != m_loadedMaterials.end()) {
        std::cout << "MaterialManager: Reusing cached Material: " << materialID << std::endl;
        return it->second;
    }

    // 2. 材质不在缓存中，尝试内部加载
    std::shared_ptr<Material> material = nullptr;
    try {
        // 调用内部辅助函数进行加载
        material = internalLoadMaterial(materialID, directory, extension, flipY);
    } catch (const std::runtime_error& e) {
        std::cerr << "MaterialManager ERROR: Failed to load Material '" << materialID << "' from directory '" << directory << "': " << e.what() << std::endl;
        return nullptr;
    } catch (const std::exception& e) { // 捕获其他可能的异常
        std::cerr << "MaterialManager ERROR: An unexpected error occurred while loading Material '" << materialID << "': " << e.what() << std::endl;
        return nullptr;
    }


    // 3. 如果加载成功，则将其添加到缓存中
    if (material) {
        m_loadedMaterials[materialID] = material;
        std::cout << "MaterialManager: Loaded and cached Material: " << materialID << std::endl;
    } else {
        std::cerr << "MaterialManager ERROR: internalLoadMaterial returned nullptr for: " << materialID << std::endl;
    }
    return material;
}

// 尝试从缓存中获取一个已加载的材质
std::shared_ptr<Material> MaterialManager::getMaterial(const std::string& materialID) {
    auto it = m_loadedMaterials.find(materialID);
    if (it != m_loadedMaterials.end()) {
        return it->second;
    }
    return nullptr; // 未找到
}

// 卸载指定 ID 的材质
void MaterialManager::unloadMaterial(const std::string& materialID) {
    // std::map::erase 返回被删除元素的数量，如果 > 0 表示成功删除
    if (m_loadedMaterials.erase(materialID) > 0) {
        std::cout << "MaterialManager: Unloaded Material: " << materialID << std::endl;
    } else {
        std::cout << "MaterialManager: Material not found in cache for unload: " << materialID << std::endl;
    }
    // 注意：当 shared_ptr 从 map 中移除时，如果其引用计数变为 0，
    // 材质对象（及其内部持有的纹理 shared_ptr）将自动析构并释放资源。
}

// 清空所有已缓存的材质
void MaterialManager::clearAllMaterials() {
    m_loadedMaterials.clear(); // 清空 map，所有 shared_ptr 将被销毁
    std::cout << "MaterialManager: All materials cleared from cache." << std::endl;
    // shared_ptr 的析构机制将确保相关资源被正确释放。
}

// --- 内部加载辅助函数实现 ---
// 这个函数封装了调用 MaterialFactory 来创建材质的逻辑
std::shared_ptr<Material> MaterialManager::internalLoadMaterial(
    const std::string& materialID,
    const std::string& directory,
    const std::string& extension,
    bool flipY)
{
    // MaterialFactory 承担了从文件系统加载纹理并创建 Material 对象的职责。
    // MaterialManager 只是协调这个过程，并进行缓存。
    return MaterialFactory::CreateMaterialFromDirectory(materialID, directory, extension, flipY);
}
