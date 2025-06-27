#include "Mesh.h"
#include <iostream>
#include <Eigen/Geometry> // 确保包含 Eigen::Affine3f

// 修改后的 Mesh 构造函数
Mesh::Mesh(const std::string& name, 
           const std::vector<Vertex>& vertices, 
           const std::vector<unsigned int>& indices, 
           std::shared_ptr<Material> material)
    : name_(name),
      vertices_(vertices), 
      indices_(indices), 
      position_(Eigen::Vector3f::Zero()), // 默认局部位置为原点
      rotation_(Eigen::Quaternionf::Identity()), // 默认无局部旋转
      scale_(Eigen::Vector3f::Ones()), // 默认无局部缩放
      modelMatrix_(Eigen::Matrix4f::Identity()), // 默认局部模型矩阵为单位矩阵
      material_(material),
      localAABB_() 
{
    // 计算这个 Mesh 的局部 AABB
    if (!vertices_.empty()) {
        for (const auto& vert : vertices_) {
            localAABB_.Extend(vert.position);
        }
    } else {
        std::cerr << "WARNING::Mesh::Mesh '" << name_ << "' has no vertices for AABB calculation." << std::endl;
    }

    if (vertices_.empty() || indices_.empty()) {
        std::cerr << "WARNING::Mesh::Mesh '" << name_ << "' has no vertices or indices. Skipping GL setup." << std::endl;
        return; 
    }
    
    setupMesh();
    updateModelMatrix(); // 初始更新局部模型矩阵
}

// ~Mesh() { /* unique_ptr 会自动管理，无需手动 delete */ }

void Mesh::setupMesh() {
    VAO_ = std::make_unique<VertexArray>();
    VBO_ = std::make_unique<VertexBuffer>(vertices_.data(), vertices_.size() * sizeof(Vertex), GL_STATIC_DRAW);
    EBO_ = std::make_unique<IndexBuffer>(indices_.data(), indices_.size(), GL_STATIC_DRAW);

    VAO_->bind();
    VAO_->setIndexBuffer(*EBO_);

    VAO_->setAttribute(0, *VBO_, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position), sizeof(Vertex), 0);
    VAO_->setAttribute(1, *VBO_, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal), sizeof(Vertex), 0);
    VAO_->setAttribute(2, *VBO_, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texCoords), sizeof(Vertex), 0);
    VAO_->setAttribute(3, *VBO_, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, tangent), sizeof(Vertex), 0);
    VAO_->setAttribute(4, *VBO_, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, bitangent), sizeof(Vertex), 0);

    VAO_->unbind();
}

void Mesh::render(Shader &shader) const {
    shader.use(); // 确保着色器被激活

    // !!! 关键改动 !!!
    // Mesh 不再重新设置 "model" Uniform。
    // Model 在调用 Mesh::render 之前，已经设置了 Model 自身的 modelMatrix 到 "model" Uniform。
    // 现在，Mesh 需要将自己的局部变换（modelMatrix_）作为一个单独的 Uniform 传递。
    // 你的着色器需要修改，以处理两个矩阵相乘：
    // final_model_matrix = Model_Matrix * Mesh_Local_Matrix;
    // gl_Position = projection * view * final_model_matrix * vec4(position, 1.0);

    // 如果你的着色器中，"model" Uniform 仍然是 Model 的世界矩阵，
    // 那么 Mesh 的局部变换就应该被单独传递。
    // 假设着色器中有一个新的 uniform "meshLocalTransform"
    shader.setMat4("meshLocalTransform", modelMatrix_); 
    
    // 如果你没有 "meshLocalTransform" 这样的 uniform，
    // 并且希望 Mesh 的 modelMatrix_ 能够被直接应用
    // 那么你需要确保 Model 在调用每个 Mesh 的 render 时，
    // 要么：
    //   1. 传递组合好的 Model * Mesh_Local 矩阵到着色器的 "model" Uniform
    //   2. 或者，Mesh 在自己的 render 中，将自身的 modelMatrix_ 与 Model 传递进来的矩阵相乘，
    //      然后将结果传到着色器的 "model" Uniform。
    //      这通常需要在 Mesh::render 接受一个 const Eigen::Matrix4f& parentModelMatrix 参数。

    // ** 最简单的兼容现有着色器的方式（但不是最优雅的层级）：**
    // ** 如果你当前的着色器只有一个 "model" uniform，且期望它是世界变换矩阵，**
    // ** 那么 Model::render 应该计算好 Model * Mesh_Local 然后传给 shader。**
    // ** 但是，考虑到 Model 已经设置了 model uniform，这里 Mesh 不再设置，**
    // ** 而是假设着色器已经有了 Model 的全局变换，并会结合 Mesh 的局部变换。**
    // ** 为了保持 Mesh::render 的独立性，且不依赖父级传递矩阵，**
    // ** 我们需要在着色器中组合。**

    // 绑定材质的纹理和 Uniforms
    if (material_) {
        material_->bindTextures(shader);
        material_->setUniforms(shader);
    }

    VAO_->bind();
    glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0); 
    VAO_->unbind();
}

Eigen::Matrix4f Mesh::getModelMatrix() const {
    return modelMatrix_;
}

void Mesh::setModelMatrix(const Eigen::Matrix4f &modelMatrix) {
    modelMatrix_ = modelMatrix;
    // 注意：如果直接设置了 modelMatrix_，那么 position_, rotation_, scale_ 将不再与它同步。
    // 如果需要同步，你需要从 modelMatrix_ 反推它们，这会比较复杂。
}

Eigen::Vector3f Mesh::getPosition() const {
    return position_;
}

void Mesh::setPosition(const Eigen::Vector3f &position) {
    position_ = position;
    updateModelMatrix();
}

Eigen::Quaternionf Mesh::getRotation() const {
    return rotation_;
}

void Mesh::setRotation(const Eigen::Quaternionf &rotation) {
    rotation_ = rotation;
    rotation_.normalize(); // 归一化四元数
    updateModelMatrix();
}

Eigen::Vector3f Mesh::getScale() const {
    return scale_;
}

void Mesh::setScale(const Eigen::Vector3f &scale) {
    scale_ = scale;
    updateModelMatrix();
}

void Mesh::updateModelMatrix() {
    // 构造平移矩阵
    Eigen::Matrix4f translationMatrix = Eigen::Matrix4f::Identity();
    translationMatrix.block<3, 1>(0, 3) = position_;

    // 构造旋转矩阵
    Eigen::Matrix4f rotationMatrix = Eigen::Matrix4f::Identity();
    rotationMatrix.block<3, 3>(0, 0) = rotation_.toRotationMatrix();

    // 构造缩放矩阵
    Eigen::Matrix4f scaleMatrix = Eigen::Matrix4f::Identity();
    scaleMatrix(0, 0) = scale_.x();
    scaleMatrix(1, 1) = scale_.y();
    scaleMatrix(2, 2) = scale_.z();

    // 组合变换矩阵：缩放 -> 旋转 -> 平移
    // 这个 modelMatrix_ 现在代表 Mesh 自身相对于其父（Model）的局部变换
    modelMatrix_ = translationMatrix * rotationMatrix * scaleMatrix;
}

// 获取世界空间 AABB 的实现
// 现在返回 unique_ptr<AABB> 避免内存泄漏问题
std::unique_ptr<AABB> Mesh::getWorldAABB(const Eigen::Matrix4f& parentWorldMatrix) const {
    // 组合 Mesh 自身的局部变换和父 Model 的世界变换
    Eigen::Matrix4f meshWorldTransform = parentWorldMatrix * modelMatrix_; 
    // 直接返回 localAABB_.Transform 返回的 unique_ptr
    return localAABB_.Transform(meshWorldTransform);
}