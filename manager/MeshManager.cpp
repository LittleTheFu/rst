#include "MeshManager.h"
#include <stdexcept> // For std::runtime_error
#include <functional> // For std::hash (if used for content hashing)

// Static member initialization
MeshManager& MeshManager::getInstance() {
    static MeshManager instance;
    return instance;
}

MeshManager::MeshManager() : m_nextMeshId(1) { // 从 1 开始分配 ID，0 通常作为无效 ID
    std::cout << "MeshManager: Initialized." << std::endl;
}

MeshManager::~MeshManager() {
    clearAllMeshGLData();
    std::cout << "MeshManager: Shut down. All cached MeshGLData released." << std::endl;
}

// 加载或获取 MeshGLData
unsigned int MeshManager::loadMeshGLData(const std::vector<Vertex>& vertices, 
                                         const std::vector<unsigned int>& indices,
                                         const std::string& meshName) 
{
    // 优先使用提供的名称作为查找 key
    std::string effectiveMeshName = meshName;
    if (effectiveMeshName.empty()) {
        // 如果没有提供名称，生成一个通用名称作为缓存 ID
        effectiveMeshName = "auto_gen_mesh_" + std::to_string(m_nextMeshId); 
    }

    // 1. 检查是否已通过名称缓存
    auto name_it = m_meshNameToId.find(effectiveMeshName);
    if (name_it != m_meshNameToId.end()) {
        std::cout << "MeshManager: Reusing cached MeshGLData by name: " << effectiveMeshName 
                  << " (ID: " << name_it->second << ")" << std::endl;
        return name_it->second;
    }

    // 2. 网格未缓存，创建新的 MeshGLData
    unsigned int newMeshId = m_nextMeshId++;
    std::shared_ptr<MeshGLData> meshGLData = nullptr;

    try {
        meshGLData = std::make_shared<MeshGLData>(vertices, indices);
    } catch (const std::runtime_error& e) {
        std::cerr << "MeshManager ERROR: Failed to create MeshGLData for '" << effectiveMeshName << "': " << e.what() << std::endl;
        return 0; // 返回无效 ID
    } catch (const std::exception& e) {
        std::cerr << "MeshManager ERROR: An unexpected error occurred creating MeshGLData for '" << effectiveMeshName << "': " << e.what() << std::endl;
        return 0; // 返回无效 ID
    }

    if (meshGLData) {
        // 3. 缓存新的 MeshGLData
        m_loadedMeshesById[newMeshId] = meshGLData;
        m_meshNameToId[effectiveMeshName] = newMeshId; // 关联名称和 ID

        std::cout << "MeshManager: Loaded and cached new MeshGLData (ID: " << newMeshId 
                  << ", Name: " << effectiveMeshName << ")" << std::endl;
        return newMeshId;
    } else {
        std::cerr << "MeshManager ERROR: Failed to create MeshGLData (returned nullptr) for: " << effectiveMeshName << std::endl;
        return 0; // 返回无效 ID
    }
}

// 根据 ID 获取 MeshGLData
std::shared_ptr<MeshGLData> MeshManager::getMeshGLData(unsigned int meshId) const {
    auto it = m_loadedMeshesById.find(meshId);
    if (it != m_loadedMeshesById.end()) {
        return it->second;
    }
    return nullptr; // 未找到
}

// 根据名称获取 MeshGLData
std::shared_ptr<MeshGLData> MeshManager::getMeshGLData(const std::string& name) const {
    auto name_it = m_meshNameToId.find(name);
    if (name_it != m_meshNameToId.end()) {
        return getMeshGLData(name_it->second); // 通过 ID 获取
    }
    return nullptr; // 未找到
}

// 卸载指定 ID 的 MeshGLData
void MeshManager::unloadMeshGLData(unsigned int meshId) {
    auto it = m_loadedMeshesById.find(meshId);
    if (it != m_loadedMeshesById.end()) {
        std::string meshNameFound;
        // 查找对应的名称以便从 m_meshNameToId 中移除
        for (const auto& pair : m_meshNameToId) {
            if (pair.second == meshId) {
                meshNameFound = pair.first;
                break;
            }
        }
        
        m_loadedMeshesById.erase(it);
        if (!meshNameFound.empty()) {
            m_meshNameToId.erase(meshNameFound);
        }
        std::cout << "MeshManager: Unloaded MeshGLData ID: " << meshId 
                  << (meshNameFound.empty() ? "" : " (Name: " + meshNameFound + ")") << std::endl;
    } else {
        std::cout << "MeshManager: MeshGLData ID not found in cache for unload: " << meshId << std::endl;
    }
}

// 卸载指定名称的 MeshGLData
void MeshManager::unloadMeshGLData(const std::string& meshName) {
    auto name_it = m_meshNameToId.find(meshName);
    if (name_it != m_meshNameToId.end()) {
        unsigned int meshId = name_it->second;
        m_loadedMeshesById.erase(meshId); // 移除实际的 MeshGLData
        m_meshNameToId.erase(name_it);    // 移除名称到 ID 的映射
        std::cout << "MeshManager: Unloaded MeshGLData Name: " << meshName 
                  << " (ID: " << meshId << ")" << std::endl;
    } else {
        std::cout << "MeshManager: MeshGLData Name not found in cache for unload: " << meshName << std::endl;
    }
}

// 清空所有已缓存的 MeshGLData
void MeshManager::clearAllMeshGLData() {
    m_loadedMeshesById.clear();
    m_meshNameToId.clear();
    m_nextMeshId = 1; // 重置 ID 生成器
    std::cout << "MeshManager: All MeshGLData cleared from cache." << std::endl;
}

// 示例：如果需要实现内容哈希来检测重复网格数据
// std::string MeshManager::generateMeshHash(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) const {
//     // 警告：这是一个非常简化的哈希实现，实际项目中可能需要更复杂的哈希算法
//     // 来确保不同但内容相同的网格能被识别为同一个。
//     // 例如，可以使用 Boost.Hash 或自定义的 FNV-1a 哈希。
//     std::hash<float> float_hasher;
//     std::hash<unsigned int> int_hasher;
//     size_t hash_val = 0;
//     for (const auto& v : vertices) {
//         hash_val ^= float_hasher(v.position.x()) + 0x9e3779b9 + (hash_val << 6) + (hash_val >> 2);
//         // ... hash other vertex attributes
//     }
//     for (const auto& i : indices) {
//         hash_val ^= int_hasher(i) + 0x9e3779b9 + (hash_val << 6) + (hash_val >> 2);
//     }
//     return std::to_string(hash_val);
// }