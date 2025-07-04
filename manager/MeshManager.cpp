// manager/MeshManager.cpp
#include "MeshManager.h"
#include <iostream>
#include <stdexcept> // For std::out_of_range

MeshManager::MeshManager() {
  std::cout << "INFO::MeshManager::MeshManager: MeshManager created." << std::endl;
}

MeshManager::~MeshManager() {
    Clear(); // 析构时清理所有资源
}

unsigned int MeshManager::LoadMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    // 可以添加哈希或者文件路径来检查是否已加载，避免重复
    // 但目前我们假设每次调用都创建一个新MeshGLData

    if (vertices.empty() || indices.empty()) {
        std::cerr << "WARNING::MeshManager::LoadMesh: Attempted to load empty mesh. Returning INVALID_MESH_ID." << std::endl;
        return INVALID_MESH_ID;
    }

    unsigned int newId = nextMeshId_++;
    meshes_[newId] = std::make_unique<MeshGLData>(vertices, indices);
    
    std::cout << "INFO::MeshManager::LoadMesh: Loaded mesh with ID " << newId << std::endl;
    return newId;
}

const MeshGLData& MeshManager::GetMesh(unsigned int meshId) const {
    auto it = meshes_.find(meshId);
    if (it != meshes_.end()) {
        return *(it->second);
    }
    // 抛出异常或返回默认/空对象，取决于错误处理策略
    std::cerr << "ERROR::MeshManager::GetMesh: Mesh with ID " << meshId << " not found!" << std::endl;
    // 为了避免崩溃，这里可以返回一个静态的空MeshGLData或者抛出异常
    // 这里选择抛出异常，因为这意味着逻辑错误，应该被捕获
    throw std::out_of_range("Mesh with ID not found"); 
}

bool MeshManager::HasMesh(unsigned int meshId) const {
    return meshes_.count(meshId) > 0;
}

void MeshManager::UnloadMesh(unsigned int meshId) {
    auto it = meshes_.find(meshId);
    if (it != meshes_.end()) {
        std::cout << "INFO::MeshManager::UnloadMesh: Unloading mesh with ID " << meshId << std::endl;
        meshes_.erase(it); // unique_ptr 会自动释放资源
    } else {
        std::cerr << "WARNING::MeshManager::UnloadMesh: Attempted to unload non-existent mesh with ID " << meshId << std::endl;
    }
}

void MeshManager::Clear() {
    std::cout << "INFO::MeshManager::Clear: Clearing all loaded meshes." << std::endl;
    meshes_.clear(); // unique_ptr 容器清理时会自动释放所有资源
    nextMeshId_ = 1; // 重置ID计数器
}

MeshManager &MeshManager::GetInstance()
{
    static MeshManager instance;
    return instance;
}