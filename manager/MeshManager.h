// manager/MeshManager.h
#ifndef MESH_MANAGER_H
#define MESH_MANAGER_H

#include <unordered_map>
#include <memory> // for std::unique_ptr
#include <string> // for path/name if you want to map strings to IDs
#include "mesh/MeshGLData.h" // 包含我们重构后的MeshGLData
#include "Vertex.h" // 包含Vertex结构体，因为LoadMesh需要它

// 定义一个无效的Mesh ID常量
const unsigned int INVALID_MESH_ID = 0; // 0 通常作为无效ID

class MeshManager {
public:
    MeshManager();
    ~MeshManager();

    // 加载网格数据并存储，返回一个唯一的ID。
    // 如果是基于文件路径的缓存，可以添加 path 参数。
    // 但因为MeshGLData是纯数据，这里直接接受顶点/索引数据。
    // 这个方法应该由 ModelManager 或其他更高层加载器调用。
    unsigned int LoadMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);

    // 根据ID获取网格的OpenGL数据，用于渲染
    // 返回 const 引用，确保外部不能修改内部资源
    const MeshGLData& GetMesh(unsigned int meshId) const;

    // 检查是否已加载某个网格
    bool HasMesh(unsigned int meshId) const;

    // 卸载网格资源 (释放OpenGL资源并从管理器中移除)
    void UnloadMesh(unsigned int meshId);

    // 清理所有已加载的网格资源
    void Clear();

    // 单例模式访问点
    static MeshManager& GetInstance();

private:
    std::unordered_map<unsigned int, std::unique_ptr<MeshGLData>> meshes_;
    unsigned int nextMeshId_ = 1; // 从1开始，0保留为INVALID_MESH_ID

    // 单例模式：禁止拷贝构造和赋值操作
    MeshManager(const MeshManager&) = delete;
    MeshManager& operator=(const MeshManager&) = delete;
};

#endif // MESH_MANAGER_H