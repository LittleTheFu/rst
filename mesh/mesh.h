#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include <memory> 

// OpenGL 头文件
#include <glad/glad.h>

// Assimp 头文件 (仅用于辅助转换函数)
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/vector3.h> 

// Eigen 头文件
#include <Eigen/Dense>
#include <Eigen/Geometry> 

// 你的其他辅助类
#include "shader.h"
#include "Renderable.h"
#include "Transformable.h"
#include "nameable.h" // 添加 INamable 接口
#include "Material.h"
#include "vertex.h"
#include "vertexArray.h"
#include "indexBuffer.h"
#include "vertexBuffer.h"
#include "BoundingVolume.h" 


// Mesh 类现在继承 IRenderable, ITransformable, 和 INamable
class Mesh : public IRenderable, public ITransformable, public INamable {
public:
    // 修改构造函数：现在它直接接收已经解析好的顶点、索引和材质
    Mesh(const std::string& name, 
         const std::vector<Vertex>& vertices, 
         const std::vector<unsigned int>& indices, 
         std::shared_ptr<Material> material = nullptr);
    
    // ~Mesh() 由 unique_ptr 自动管理，通常不需要显式析构函数

    // --- IRenderable 接口实现 ---
    // 渲染方法现在接受一个可选的父模型矩阵。
    // 如果没有提供（例如独立渲染 Mesh），则使用自身的 modelMatrix_
    void render(Shader& shader) const override; // 这个override是必须的

    // --- ITransformable 接口实现 ---
    // Mesh 自身的局部变换
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

    // Mesh 特有的公共方法
    std::shared_ptr<Material> getMaterial() const { return material_; }
    void setMaterial(const std::shared_ptr<Material>& material) { material_ = material; }

    const AABB& getLocalAABB() const { return localAABB_; }
    // 注意：getWorldAABB() 返回的 AABB* 需要手动 delete
    // 更好的做法是返回 unique_ptr<AABB> 或直接返回 AABB 对象 (如果它可拷贝)
    std::unique_ptr<AABB> getWorldAABB(const Eigen::Matrix4f& parentWorldMatrix) const; 

private:
    std::vector<Vertex> vertices_; 
    std::vector<unsigned int> indices_; 

    // OpenGL 缓冲区对象
    std::unique_ptr<VertexArray> VAO_;
    std::unique_ptr<VertexBuffer> VBO_;
    std::unique_ptr<IndexBuffer> EBO_;

    // Mesh 自身的属性
    std::shared_ptr<Material> material_;
    Eigen::Vector3f position_;     // Mesh 相对其父节点（通常是Model）的局部位置
    Eigen::Quaternionf rotation_;  // Mesh 相对其父节点（通常是Model）的局部旋转
    Eigen::Vector3f scale_;        // Mesh 相对其父节点（通常是Model）的局部缩放
    Eigen::Matrix4f modelMatrix_;  // Mesh 自身的局部变换矩阵 (Translation * Rotation * Scale)

    AABB localAABB_; // 存储这个 Mesh 的局部空间 AABB

    // 私有辅助函数
    void setupMesh();          // 负责设置VAO, VBO, EBO
    void updateModelMatrix();  // 负责更新 modelMatrix_，基于 position_, rotation_, scale_

    const std::string name_; // Mesh 的名称 (通常来自 Assimp 的 aiMesh->mName)
};

#endif // MESH_H