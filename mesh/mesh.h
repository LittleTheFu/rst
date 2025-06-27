#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include <memory>

#include <glad/glad.h>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/vector3.h>

#include <Eigen/Dense>
#include <Eigen/Geometry>

#include "shader.h"
#include "Renderable.h"
#include "Transformable.h"
#include "nameable.h"
#include "Material.h"
#include "vertex.h"
#include "vertexArray.h"
#include "indexBuffer.h"
#include "vertexBuffer.h"
#include "BoundingVolume.h"

class Mesh : public IRenderable, public ITransformable, public INamable
{
public:
    Mesh(const std::string &name,
         const std::vector<Vertex> &vertices,
         const std::vector<unsigned int> &indices,
         std::shared_ptr<Material> material = nullptr);

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

    std::shared_ptr<Material> getMaterial() const { return material_; }
    void setMaterial(const std::shared_ptr<Material> &material) { material_ = material; }

    const AABB &getLocalAABB() const { return localAABB_; }

    std::unique_ptr<AABB> getWorldAABB(const Eigen::Matrix4f &parentWorldMatrix) const;

private:
    std::vector<Vertex> vertices_;
    std::vector<unsigned int> indices_;

    std::unique_ptr<VertexArray> VAO_;
    std::unique_ptr<VertexBuffer> VBO_;
    std::unique_ptr<IndexBuffer> EBO_;

    std::shared_ptr<Material> material_;
    Eigen::Vector3f position_;
    Eigen::Quaternionf rotation_;
    Eigen::Vector3f scale_;
    Eigen::Matrix4f modelMatrix_;

    AABB localAABB_;

    void setupMesh();
    void updateModelMatrix();

    const std::string name_;
};

#endif