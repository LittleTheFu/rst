#ifndef MESH_MANAGER_H
#define MESH_MANAGER_H

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <iostream> // For logging

#include "MeshGLData.h" // 包含我们之前定义的 MeshGLData 类
#include "Vertex.h"     // 包含 Vertex 结构体

/**
 * @brief MeshManager 是一个单例类，负责管理所有 MeshGLData 实例。
 * * 它确保每个独特的网格数据（VAO/VBO/EBO）只在 GPU 上加载一次，
 * 并提供一个唯一的 ID (meshId) 来引用这些数据。
 */
class MeshManager {
public:
    /**
     * @brief 获取 MeshManager 的唯一实例。
     * @return MeshManager 实例的引用。
     */
    static MeshManager& getInstance();

    // --- 公共加载/获取接口 ---

    /**
     * @brief 加载或获取一个 MeshGLData 实例。
     * * 如果具有相同顶点和索引数据的 MeshGLData 已在缓存中，则返回其 ID；
     * 否则，创建新的 MeshGLData，缓存并返回新 ID。
     * * @param vertices 构成网格的顶点数据。
     * @param indices 构成网格的索引数据。
     * @param meshName 可选：网格的名称，用于生成缓存 ID。如果为空，将生成一个通用 ID。
     * @return 分配给 MeshGLData 的唯一 ID。如果加载失败，返回 0 (通常 0 表示无效ID)。
     */
    unsigned int loadMeshGLData(const std::vector<Vertex>& vertices, 
                                const std::vector<unsigned int>& indices,
                                const std::string& meshName = "");

    /**
     * @brief 根据 ID 获取 MeshGLData。
     * @param meshId 要获取的 MeshGLData 的 ID。
     * @return MeshGLData 的共享指针，如果未找到则返回 nullptr。
     */
    std::shared_ptr<MeshGLData> getMeshGLData(unsigned int meshId) const;

    /**
     * @brief 根据名称（用于缓存）获取 MeshGLData。
     * @param name 用于缓存的网格名称。
     * @return MeshGLData 的共享指针，如果未找到则返回 nullptr。
     * @note 此方法要求 loadMeshGLData 时提供了唯一的名称。
     */
    std::shared_ptr<MeshGLData> getMeshGLData(const std::string& name) const;


    /**
     * @brief 卸载指定 ID 的 MeshGLData。
     * * 这将从缓存中移除 MeshGLData。如果它是最后一个引用，
     * MeshGLData 对象本身（及其内部的 OpenGL 资源）将被释放。
     * @param meshId 要卸载 MeshGLData 的 ID。
     */
    void unloadMeshGLData(unsigned int meshId);

    /**
     * @brief 卸载指定名称的 MeshGLData。
     * @param meshName 要卸载 MeshGLData 的名称。
     */
    void unloadMeshGLData(const std::string& meshName);

    /**
     * @brief 清空所有已缓存的 MeshGLData。
     * * 强制释放所有缓存的 MeshGLData。MeshManager 自身不持有资源的唯一所有权，
     * 但清空 map 会减少 shared_ptr 的引用计数。如果外部没有其他引用，资源将被释放。
     */
    void clearAllMeshGLData();

private:
    // --- 单例模式的私有成员 ---
    MeshManager();                                   // 私有构造函数
    ~MeshManager();                                  // 私有析构函数
    MeshManager(const MeshManager&) = delete;            // 禁用拷贝构造函数
    MeshManager& operator=(const MeshManager&) = delete;  // 禁用赋值运算符

    // --- 内部网格缓存 ---
    // 映射 meshId 到 MeshGLData 实例
    std::map<unsigned int, std::shared_ptr<MeshGLData>> m_loadedMeshesById;
    // 映射 meshName 到 meshId (便于通过名称查找)
    std::map<std::string, unsigned int> m_meshNameToId;

    unsigned int m_nextMeshId; // 用于生成新的唯一 meshId

    // 内部辅助函数，用于生成 MeshGLData 的唯一哈希字符串（用于命名或查找重复）
    // 注意：实际的哈希计算可能非常复杂且性能敏感，这里仅作示意。
    // 更健壮的方案可能需要对顶点和索引数据进行实际的哈希。
    // 在这里，我们将简化为直接使用 meshName 或生成递增ID。
    // std::string generateMeshHash(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) const;
};

#endif // MESH_MANAGER_H