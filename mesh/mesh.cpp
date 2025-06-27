#include "Mesh.h"
#include <iostream>
#include <Eigen/Geometry>

Mesh::Mesh(const std::string &name,
           const std::vector<Vertex> &vertices,
           const std::vector<unsigned int> &indices,
           std::shared_ptr<Material> material)
    : name_(name),
      vertices_(vertices),
      indices_(indices),
      position_(Eigen::Vector3f::Zero()),
      rotation_(Eigen::Quaternionf::Identity()),
      scale_(Eigen::Vector3f::Ones()),
      modelMatrix_(Eigen::Matrix4f::Identity()),
      material_(material),
      localAABB_()
{

    if (!vertices_.empty())
    {
        for (const auto &vert : vertices_)
        {
            localAABB_.Extend(vert.position);
        }
    }
    else
    {
        std::cerr << "WARNING::Mesh::Mesh '" << name_ << "' has no vertices for AABB calculation." << std::endl;
    }

    if (vertices_.empty() || indices_.empty())
    {
        std::cerr << "WARNING::Mesh::Mesh '" << name_ << "' has no vertices or indices. Skipping GL setup." << std::endl;
        return;
    }

    setupMesh();
    updateModelMatrix();
}

void Mesh::setupMesh()
{
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

void Mesh::render(Shader &shader) const
{
    shader.use();

    shader.setMat4("meshLocalTransform", modelMatrix_);

    if (material_)
    {
        material_->bindTextures(shader);
        material_->setUniforms(shader);
    }

    VAO_->bind();
    glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
    VAO_->unbind();
}

Eigen::Matrix4f Mesh::getModelMatrix() const
{
    return modelMatrix_;
}

void Mesh::setModelMatrix(const Eigen::Matrix4f &modelMatrix)
{
    modelMatrix_ = modelMatrix;
}

Eigen::Vector3f Mesh::getPosition() const
{
    return position_;
}

void Mesh::setPosition(const Eigen::Vector3f &position)
{
    position_ = position;
    updateModelMatrix();
}

Eigen::Quaternionf Mesh::getRotation() const
{
    return rotation_;
}

void Mesh::setRotation(const Eigen::Quaternionf &rotation)
{
    rotation_ = rotation;
    rotation_.normalize();
    updateModelMatrix();
}

Eigen::Vector3f Mesh::getScale() const
{
    return scale_;
}

void Mesh::setScale(const Eigen::Vector3f &scale)
{
    scale_ = scale;
    updateModelMatrix();
}

void Mesh::updateModelMatrix()
{

    Eigen::Matrix4f translationMatrix = Eigen::Matrix4f::Identity();
    translationMatrix.block<3, 1>(0, 3) = position_;

    Eigen::Matrix4f rotationMatrix = Eigen::Matrix4f::Identity();
    rotationMatrix.block<3, 3>(0, 0) = rotation_.toRotationMatrix();

    Eigen::Matrix4f scaleMatrix = Eigen::Matrix4f::Identity();
    scaleMatrix(0, 0) = scale_.x();
    scaleMatrix(1, 1) = scale_.y();
    scaleMatrix(2, 2) = scale_.z();

    modelMatrix_ = translationMatrix * rotationMatrix * scaleMatrix;
}

std::unique_ptr<AABB> Mesh::getWorldAABB(const Eigen::Matrix4f &parentWorldMatrix) const
{

    Eigen::Matrix4f meshWorldTransform = parentWorldMatrix * modelMatrix_;

    return localAABB_.Transform(meshWorldTransform);
}