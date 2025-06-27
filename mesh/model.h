#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include <memory>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "sceneObject.h" // 包含 ISceneObject 接口
#include "Mesh.h"         // 包含你的 Mesh 类
#include "shader.h"       // 包含 Shader
#include "Material.h"     // 包含 Material
#include "AssimpUtils.h"  // 包含 Assimp 辅助转换函数

// Model 类现在继承 ISceneObject，因此它将实现 IRenderable, ITransformable, INamable 的所有方法
class Model : public ISceneObject
{
public:
    // 构造函数：接受模型文件的路径
    Model(const std::string& path);
    // 析构函数 (如果需要特殊清理)
    // ~Model(); // 智能指针会自动管理，通常不需要显式析构函数

    // --- IRenderable 接口实现 ---
    void render(Shader& shader) const override;

    // --- ITransformable 接口实现 ---
    Eigen::Matrix4f getModelMatrix() const override;
    void setModelMatrix(const Eigen::Matrix4f& modelMatrix) override;
    Eigen::Vector3f getPosition() const override;
    void setPosition(const Eigen::Vector3f& position) override;
    Eigen::Quaternionf getRotation() const override;
    void setRotation(const Eigen::Quaternionf& rotation) override;
    Eigen::Vector3f getScale() const override;
    void setScale(const Eigen::Vector3f& scale) override;

    // --- INamable 接口实现 ---
    const std::string& getName() const override { return name_; }


    std::unique_ptr<AABB> getWorldAABB() const;

    // Model 特有的方法
    // 设置 Model 中所有 Mesh 的材质（谨慎使用，通常材质是每个 Mesh 独立的）
    void setMaterial(std::shared_ptr<Material> material); 
    
    // 获取所有子 Mesh (const 引用，不允许外部修改 Model 内部的 Mesh 列表)
    const std::vector<std::unique_ptr<Mesh>>& getMeshes() const { return meshes_; }

    // quick and dirty: 从 Model 中取出指定索引的 Mesh 的所有权
    // 注意：这将从 Model 内部移除该 Mesh，使其不再由 Model 管理和渲染
    std::unique_ptr<Mesh> takeMesh(size_t index);

private:
    std::vector<std::unique_ptr<Mesh>> meshes_; // 存储模型中所有的子 Mesh
    std::string directory_;                     // 存储模型文件所在的目录
    std::string name_;                          // Model 自身的名称 (可以来自文件名，或 Assimp 根节点名称)

    // Model 自身的变换属性
    Eigen::Vector3f position_;
    Eigen::Quaternionf rotation_;
    Eigen::Vector3f scale_;
    Eigen::Matrix4f modelMatrix_; // Model 自身的局部到世界空间的变换矩阵

    // 辅助函数
    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    std::unique_ptr<Mesh> processMesh(aiMesh* mesh, const aiScene* scene);
    std::shared_ptr<Material> processMaterial(aiMaterial* mat);
    std::vector<std::shared_ptr<Texture>> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);
    
    // 负责更新 modelMatrix_，每次 position/rotation/scale 改变时调用
    void updateModelMatrix();

    // 已加载纹理的缓存 (为了避免重复加载，可以放到 TextureManager 中)
    std::vector<std::shared_ptr<Texture>> textures_loaded_; 
};

#endif // MODEL_H