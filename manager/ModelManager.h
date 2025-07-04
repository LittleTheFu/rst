#ifndef MODEL_MANAGER_H
#define MODEL_MANAGER_H

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <iostream> // For logging

// Assimp includes
#include <assimp/Importer.hpp> // Assimp 导入器
#include <assimp/scene.h>      // Assimp 场景对象
#include <assimp/postprocess.h> // Assimp 后处理标志

// Project specific includes
#include "ModelResource.h"      // 我们刚刚定义的 ModelResource
#include "MeshManager.h"        // MeshManager 的头文件
#include "MaterialManager.h"    // MaterialManager 的头文件
#include "Vertex.h"             // Vertex 结构体
#include "TextureManager.h"     // 如果 Assimp 材质加载需要直接使用 TextureManager


/**
 * @brief ModelManager 是一个单例类，负责加载、缓存和管理 3D 模型资产。
 * 它使用 Assimp 库解析模型文件，并将数据组织成 ModelResource 对象。
 */
class ModelManager {
public:
    /**
     * @brief 获取 ModelManager 的唯一实例。
     * @return ModelManager 实例的引用。
     */
    static ModelManager& getInstance();

    // --- 公共加载/获取接口 ---

    /**
     * @brief 从文件加载或获取一个模型。
     * 如果模型已在缓存中，则直接返回其 ModelResource 的共享指针；
     * 否则，使用 Assimp 加载并缓存。
     *
     * @param filePath 模型的完整文件路径。
     * @param modelName 可选：模型的唯一名称。如果为空，将从文件路径生成。
     * @return ModelResource 的共享指针，如果加载失败则返回 nullptr。
     */
    std::shared_ptr<ModelResource> loadModel(const std::string& filePath, const std::string& modelName = "");

    /**
     * @brief 尝试从缓存中获取一个已加载的模型。
     * @param modelName 模型的唯一名称。
     * @return ModelResource 的共享指针，如果未找到则返回 nullptr。
     */
    std::shared_ptr<ModelResource> getModel(const std::string& modelName) const;

    /**
     * @brief 卸载指定名称的模型。
     * @param modelName 要卸载模型的名称。
     */
    void unloadModel(const std::string& modelName);

    /**
     * @brief 清空所有已缓存的模型。
     */
    void clearAllModels();

private:
    // --- 单例模式的私有成员 ---
    ModelManager();                                  // 私有构造函数
    ~ModelManager();                                 // 私有析构函数
    ModelManager(const ModelManager&) = delete;            // 禁用拷贝构造函数
    ModelManager& operator=(const ModelManager&) = delete;  // 禁用赋值运算符

    // --- 内部模型缓存 ---
    std::map<std::string, std::shared_ptr<ModelResource>> m_loadedModels;

    // --- Assimp 辅助函数 ---

    /**
     * @brief 处理 Assimp 场景中的所有节点。
     * 递归遍历节点的子节点，并处理每个节点下的网格。
     * @param node Assimp 的 aiNode 根节点。
     * @param scene Assimp 的 aiScene 对象。
     * @param modelResource 正在构建的 ModelResource 对象。
     * @param modelDirectory 模型文件所在的目录。
     * @param parentTransform 当前节点的累积变换。
     */
    void processNode(aiNode* node, const aiScene* scene, 
                     std::shared_ptr<ModelResource> modelResource, 
                     const std::string& modelDirectory,
                     const Eigen::Matrix4f& parentTransform);

    /**
     * @brief 处理 Assimp 的 aiMesh 对象，提取顶点/索引数据，并加载材质。
     * @param mesh Assimp 的 aiMesh 对象。
     * @param scene Assimp 的 aiScene 对象。
     * @param modelResource 正在构建的 ModelResource 对象。
     * @param modelDirectory 模型文件所在的目录。
     * @param meshTransform 该网格的局部变换（来自节点）。
     * @return 成功处理返回 true，否则返回 false。
     */
    bool processMesh(aiMesh* mesh, const aiScene* scene, 
                     std::shared_ptr<ModelResource> modelResource, 
                     const std::string& modelDirectory,
                     const Eigen::Matrix4f& meshTransform);

    // 辅助函数：将 Assimp 的 aiMatrix4x4 转换为 Eigen::Matrix4f
    Eigen::Matrix4f convertMatrixToEigen(const aiMatrix4x4& from);
    // 辅助函数：从文件路径中提取目录
    std::string getDirectoryPath(const std::string& filePath);
};

#endif // MODEL_MANAGER_H