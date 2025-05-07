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
#include "Texture.h" // 包含 Texture 类

struct Vertex {
    Eigen::Vector3f position;
    Eigen::Vector3f normal;
    Eigen::Vector2f texCoords;
};

class Mesh : public Renderable, public Transformable {
public:
    Mesh(const std::string& filePath);
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

private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    GLuint VAO, VBO, EBO;
    Texture diffuseTexture;
    Eigen::Vector3f position_;
    Eigen::Quaternionf rotation_;
    Eigen::Vector3f scale_;
    Eigen::Matrix4f modelMatrix_;

    void setupMesh();
    void updateModelMatrix();
};

#endif // MESH_H