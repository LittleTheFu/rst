#ifndef MESH_H
#define MESH_H

#include <vector>
#include <glad/glad.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Eigen/Dense>
#include <Eigen/Geometry> // For Eigen::Quaternionf and Eigen::Affine3f
#include "shader.h"
#include "Renderable.h"     // 包含 Renderable 接口
#include "Transformable.h"  // 包含 Transformable 接口
#include "Material.h"       // 包含 Material 类
#include <memory>           // 包含 std::shared_ptr, std::unique_ptr
#include "vertex.h"         // 包含 Vertex 结构体
#include "vertexArray.h"
#include "indexBuffer.h"
#include "vertexBuffer.h"
#include "BoundingVolume.h" // 包含 AABB 类 (你之前创建的)

class Mesh : public Renderable, public Transformable {
public:
    Mesh(const std::string& filePath, std::shared_ptr<Material> material = nullptr); // 构造函数接受 Material
    ~Mesh();

    void render(Shader& shader) const override; // 实现 Renderable 接口
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

    // --- 新增 AABB 相关接口 ---
    /**
     * @brief 获取模型在局部空间（原始导入尺寸）的AABB。
     * @return 模型的局部AABB。
     */
    const AABB& getLocalAABB() const { return localAABB_; }

    /**
     * @brief 获取模型在世界空间中当前的AABB。
     * 这个AABB是经过模型矩阵变换后的AABB，是进行射线拾取等操作时需要的。
     * @return 模型的当前世界空间AABB。请注意，这个函数返回的是一个新分配的 AABB*，需要手动 delete。
     */
    AABB* getWorldAABB() const;

private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    std::unique_ptr<VertexArray> VAO_;
    std::unique_ptr<VertexBuffer> VBO_;
    std::unique_ptr<IndexBuffer> EBO_;

    std::shared_ptr<Material> material_;
    Eigen::Vector3f position_;
    Eigen::Quaternionf rotation_;
    Eigen::Vector3f scale_;
    Eigen::Matrix4f modelMatrix_;

    AABB localAABB_; // 新增：存储整个 Mesh 的局部空间 AABB

    void setupMesh();
    void updateModelMatrix();

    // 辅助函数：将 Assimp 的 aiVector3D 转换为 Eigen::Vector3f
    Eigen::Vector3f ConvertAssimpVec3ToEigen(const aiVector3D& vec);
};

#endif // MESH_H