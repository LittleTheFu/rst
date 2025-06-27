#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include <memory>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h" // 包含你的 Mesh 类
#include "shader.h" // 包含 Shader
#include "Material.h" // 包含 Material

class Model {
public:
    // 构造函数：接受模型文件的路径
    Model(const std::string& path);
    // 渲染所有子 Mesh
    void render(Shader& shader) const;

    // 获取所有子 Mesh
    const std::vector<std::unique_ptr<Mesh>>& getMeshes() const { return meshes_; }

    // --- Transformable 接口 (可选) ---
    // 你可以给Model也添加Transformable接口，让整个模型作为一个整体进行变换
    // 但如果每个Mesh都有自己的变换，Model可以只作为一个加载和渲染的容器
    // 这里我们假设Model作为一个整体来管理所有Mesh，其变换直接应用到每个子Mesh

private:
    // 存储模型中所有的子 Mesh
    std::vector<std::unique_ptr<Mesh>> meshes_;
    // 存储模型文件所在的目录，用于加载相对路径的纹理
    std::string directory_;

    // Assimp 导入器对象 (不需要作为成员，在加载函数内部局部创建即可)
    // Assimp::Importer importer_;

    // --- 加载辅助函数 ---
    void loadModel(const std::string& path);
    // 处理 Assimp 场景中的节点，递归加载 Mesh
    void processNode(aiNode* node, const aiScene* scene);
    // 处理 Assimp 的 aiMesh，创建我们自己的 Mesh 对象
    std::unique_ptr<Mesh> processMesh(aiMesh* mesh, const aiScene* scene);
    // 加载材质中的纹理
    std::vector<std::shared_ptr<Texture>> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);
    // 解析 Assimp 材质到你的 Material 类
    std::shared_ptr<Material> processMaterial(aiMaterial* mat);
};

#endif // MODEL_H