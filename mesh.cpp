#include "Mesh.h"
#include <iostream>
#include <Eigen/Geometry>

Mesh::Mesh(const std::string& filePath, std::shared_ptr<Material> material) :
    position_(Eigen::Vector3f::Zero()),
    rotation_(Eigen::Quaternionf::Identity()),
    scale_(Eigen::Vector3f::Ones()),
    modelMatrix_(Eigen::Matrix4f::Identity()),
    material_(material) // 初始化 Material
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    // 假设我们只处理第一个网格
    unsigned int meshIndex = scene->mRootNode->mChildren[0]->mMeshes[0];
    aiMesh* mesh = scene->mMeshes[meshIndex];

    // 顶点数据
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        vertices.push_back({
            {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z},
            mesh->HasNormals() ? Eigen::Vector3f(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z) : Eigen::Vector3f(0.0f, 0.0f, 0.0f),
            mesh->HasTextureCoords(0) ? Eigen::Vector2f(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : Eigen::Vector2f(0.0f, 0.0f)
        });
    }

    // 索引数据
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    setupMesh();
    updateModelMatrix(); // 初始化模型矩阵
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Mesh::setupMesh() {
    glCreateVertexArrays(1, &VAO);
    glCreateBuffers(1, &VBO);
    glCreateBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // 顶点数据
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // 索引数据
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // 顶点属性指针
    // 位置
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    // 法线
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // 纹理坐标
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

    glBindVertexArray(0);
}

void Mesh::render(Shader& shader) {
    shader.use();

    // 设置模型矩阵 Uniform
    shader.setMat4("model", modelMatrix_);

    // 绑定材质的纹理和 Uniforms
    if (material_) {
        material_->bindTextures(shader);
        material_->setUniforms(shader); // 假设 Material 类有这个方法
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

Eigen::Matrix4f Mesh::getModelMatrix() const {
    return modelMatrix_;
}

void Mesh::setModelMatrix(const Eigen::Matrix4f& modelMatrix) {
    modelMatrix_ = modelMatrix;
}

Eigen::Vector3f Mesh::getPosition() const {
    return position_;
}

void Mesh::setPosition(const Eigen::Vector3f& position) {
    position_ = position;
    updateModelMatrix();
}

Eigen::Quaternionf Mesh::getRotation() const {
    return rotation_;
}

void Mesh::setRotation(const Eigen::Quaternionf& rotation) {
    rotation_ = rotation;
    updateModelMatrix();
}

Eigen::Vector3f Mesh::getScale() const {
    return scale_;
}

void Mesh::setScale(const Eigen::Vector3f& scale) {
    scale_ = scale;
    updateModelMatrix();
}

void Mesh::updateModelMatrix()
{
    Eigen::Affine3f model =
        Eigen::Translation3f(position_) *
        Eigen::AngleAxisf(rotation_) *
        Eigen::Scaling(scale_);

    modelMatrix_ = model.matrix();
}