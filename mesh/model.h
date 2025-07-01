#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include <memory>
#include <map> // 新增：用于骨骼信息映射
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "sceneObject.h"
#include "Mesh.h"
#include "shader.h"
#include "Material.h"
#include "AssimpUtils.h"
#include "Animator.h" // <-- 新增：包含Animator头文件

class Model : public ISceneObject
{
public:
    Model(const std::string &path);
    ~Model(); // 添加析构函数

    void render(Shader &shader) const override;

    // 新增：更新动画的方法
    void update(float deltaTime); 

    // 新增：播放动画的方法
    void playAnimation(const std::string& animationName);
    void stopAnimation();

    Eigen::Matrix4f getModelMatrix() const override;
    void setModelMatrix(const Eigen::Matrix4f &modelMatrix) override;
    Eigen::Vector3f getPosition() const override;
    void setPosition(const Eigen::Vector3f &position) override;
    Eigen::Quaternionf getRotation() const override;
    void setRotation(const Eigen::Quaternionf &rotation) override;
    Eigen::Vector3f getScale() const override;
    void setScale(const Eigen::Vector3f &scale) override;

    Eigen::Vector3f getLocalBoundingBoxHalfExtents() const override;

    const std::string &getName() const override { return name_; }

    std::unique_ptr<AABB> getWorldAABB() const;

    void setMaterial(std::shared_ptr<Material> material);

    const std::vector<std::unique_ptr<Mesh>> &getMeshes() const { return meshes_; }

    std::unique_ptr<Mesh> takeMesh(size_t index);

private:
    std::vector<std::unique_ptr<Mesh>> meshes_;
    std::string directory_;
    std::string name_;

    Eigen::Vector3f position_;
    Eigen::Quaternionf rotation_;
    Eigen::Vector3f scale_;
    Eigen::Matrix4f modelMatrix_;

    std::unique_ptr<Animator> animator_; // <-- 新增：Animator实例
    // 用于在加载模型时临时存储骨骼信息 (名称到ID和逆绑定姿态矩阵)
    // 传递给 Animator::Init
    std::map<std::string, int> boneInfoMap_; 
    // Assimp中的节点转换矩阵，用于构建骨骼层次
    std::map<std::string, Eigen::Matrix4f> nodeTransformMap_; 


    void loadModel(const std::string &path);
    // 修改 processNode 以传递 Assimp 节点自身的变换矩阵
    void processNode(aiNode *node, const aiScene *scene, const Eigen::Matrix4f& parentTransform); 
    // 修改 processMesh 以处理骨骼数据
    std::unique_ptr<Mesh> processMesh(aiMesh *mesh, const aiScene *scene); 
    std::shared_ptr<Material> processMaterial(aiMaterial *mat);
    std::vector<std::shared_ptr<Texture>> loadMaterialTextures(aiMaterial *mat, aiTextureType type, const std::string &typeName);

    void updateModelMatrix();

    std::vector<std::shared_ptr<Texture>> textures_loaded_;
};

#endif