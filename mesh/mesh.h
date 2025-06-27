#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include <memory> // For std::unique_ptr, std::shared_ptr

// OpenGL 头文件
#include <glad/glad.h>

// Assimp 头文件 (仅用于辅助转换函数，Mesh类本身不再直接使用Importer)
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/vector3.h> // For aiVector3D

// Eigen 头文件
#include <Eigen/Dense>
#include <Eigen/Geometry> // For Eigen::Quaternionf and Eigen::Affine3f

// 你的其他辅助类
#include "shader.h"
#include "Renderable.h"
#include "Transformable.h"
#include "Material.h"
#include "vertex.h"
#include "vertexArray.h"
#include "indexBuffer.h"
#include "vertexBuffer.h"
#include "BoundingVolume.h" // 包含 AABB 类 (你之前创建的)


class Mesh : public Renderable, public Transformable {
public:
    // 修改构造函数：现在它直接接收已经解析好的顶点、索引和材质
    Mesh(const std::string& name, 
         const std::vector<Vertex>& vertices, 
         const std::vector<unsigned int>& indices, 
         std::shared_ptr<Material> material = nullptr);
    
    ~Mesh();

    void render(Shader& shader) const override;
    Eigen::Matrix4f getModelMatrix() const override;
    void setModelMatrix(const Eigen::Matrix4f& modelMatrix) override;

    Eigen::Vector3f getPosition() const override;
    void setPosition(const Eigen::Vector3f& position) override;

    Eigen::Quaternionf getRotation() const override;
    void setRotation(const Eigen::Quaternionf& rotation) override;

    Eigen::Vector3f getScale() const override;
    void setScale(const Eigen::Vector3f& scale) override;

    std::shared_ptr<Material> getMaterial() const { return material_; }
    void setMaterial(const std::shared_ptr<Material>& material) { material_ = material; }

    const AABB& getLocalAABB() const { return localAABB_; }
    AABB* getWorldAABB() const; // 返回新分配的 AABB*，需要手动 delete

    const std::string& getName() const { return name_; }

private:
    std::vector<Vertex> vertices_; // 存储顶点数据 (为了AABB计算)
    std::vector<unsigned int> indices_; // 存储索引数据 (为了渲染)

    std::unique_ptr<VertexArray> VAO_;
    std::unique_ptr<VertexBuffer> VBO_;
    std::unique_ptr<IndexBuffer> EBO_;

    std::shared_ptr<Material> material_;
    Eigen::Vector3f position_;
    Eigen::Quaternionf rotation_;
    Eigen::Vector3f scale_;
    Eigen::Matrix4f modelMatrix_;

    AABB localAABB_; // 存储这个 Mesh 的局部空间 AABB

    void setupMesh(); // 负责设置VAO, VBO, EBO
    void updateModelMatrix();

    const std::string name_; // Mesh 的名称 (来自 Assimp 的 aiMesh->mName)
};

#endif // MESH_H