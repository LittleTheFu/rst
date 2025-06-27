#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include <memory>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "sceneObject.h"
#include "Mesh.h"
#include "shader.h"
#include "Material.h"
#include "AssimpUtils.h"

class Model : public ISceneObject
{
public:
    Model(const std::string &path);

    void render(Shader &shader) const override;

    Eigen::Matrix4f getModelMatrix() const override;
    void setModelMatrix(const Eigen::Matrix4f &modelMatrix) override;
    Eigen::Vector3f getPosition() const override;
    void setPosition(const Eigen::Vector3f &position) override;
    Eigen::Quaternionf getRotation() const override;
    void setRotation(const Eigen::Quaternionf &rotation) override;
    Eigen::Vector3f getScale() const override;
    void setScale(const Eigen::Vector3f &scale) override;

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

    void loadModel(const std::string &path);
    void processNode(aiNode *node, const aiScene *scene);
    std::unique_ptr<Mesh> processMesh(aiMesh *mesh, const aiScene *scene);
    std::shared_ptr<Material> processMaterial(aiMaterial *mat);
    std::vector<std::shared_ptr<Texture>> loadMaterialTextures(aiMaterial *mat, aiTextureType type, const std::string &typeName);

    void updateModelMatrix();

    std::vector<std::shared_ptr<Texture>> textures_loaded_;
};

#endif