#ifndef MESH_H
#define MESH_H

#include <vector>
#include <glad/glad.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Eigen/Dense>
#include "shader.h"
#include "Renderable.h" // 包含 Renderable 接口
#include "Transformable.h" // 包含 Transformable 接口
#include "Material.h" // 包含 Material 类
#include <memory>     // 包含 std::shared_ptr
#include "vertex.h"   // 包含 Vertex 结构体


class Mesh : public Renderable, public Transformable {
public:
    Mesh(const std::string& filePath, std::shared_ptr<Material> material = nullptr); // 构造函数接受 Material
    ~Mesh();

    void render(Shader& shader) override; // 实现 Renderable 接口
    Eigen::Matrix4f getModelMatrix() const override; // 实现 Transformable 接口
    void setModelMatrix(const Eigen::Matrix4f& modelMatrix) override; // 实现 Transformable 接口

    Eigen::Vector3f getPosition() const override;
    void setPosition(const Eigen::Vector3f& position) override;

    Eigen::Quaternionf getRotation() const override;
    void setRotation(const Eigen::Quaternionf& rotation) override;

    Eigen::Vector3f getScale() const override;
    void setScale(const Eigen::Vector3f& scale) override;

    std::shared_ptr<Material> getMaterial() const { return material_; }
    void setMaterial(const std::shared_ptr<Material>& material) { material_ = material; }

private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    GLuint VAO, VBO, EBO;
    std::shared_ptr<Material> material_; // 使用 Material 的智能指针
    Eigen::Vector3f position_;
    Eigen::Quaternionf rotation_;
    Eigen::Vector3f scale_;
    Eigen::Matrix4f modelMatrix_;

    void setupMesh();
    void updateModelMatrix();
};

#endif // MESH_H