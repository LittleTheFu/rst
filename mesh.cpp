#include "Mesh.h"
#include <iostream>
#include <Eigen/Geometry>
#include <assert.h>

Mesh::Mesh(const std::string& filePath, std::shared_ptr<Material> material) :
    position_(Eigen::Vector3f::Zero()),
    rotation_(Eigen::Quaternionf::Identity()),
    scale_(Eigen::Vector3f::Ones()),
    modelMatrix_(Eigen::Matrix4f::Identity()),
    material_(material) // 初始化 Material
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(filePath,
                                             aiProcess_Triangulate |
                                                 aiProcess_GenNormals |
                                                 aiProcess_CalcTangentSpace |
                                                 aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    // 假设我们只处理第一个网格
    unsigned int meshIndex = scene->mRootNode->mChildren[0]->mMeshes[0];
    aiMesh* mesh = scene->mMeshes[meshIndex];

    bool hasTangents = mesh->HasTangentsAndBitangents(); // 检查是否有切线和副切线
    assert(hasTangents);

    bool hasTextureCoords = mesh->HasTextureCoords(0); // 检查是否有纹理坐标
    assert(hasTextureCoords);

    bool hasNormals = mesh->HasNormals(); // 检查是否有法线
    assert(hasNormals);

    // 顶点数据
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        vertices.push_back({{mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z},
                            hasNormals ? Eigen::Vector3f(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z) : Eigen::Vector3f(0.0f, 0.0f, 0.0f),
                            hasTextureCoords ? Eigen::Vector2f(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : Eigen::Vector2f(0.0f, 0.0f),
                            // 这里是修正后的代码，现在会正确填充 tangent 和 bitangent
                            hasTangents ? Eigen::Vector3f(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z) : Eigen::Vector3f(0.0f, 0.0f, 0.0f),
                            hasTangents ? Eigen::Vector3f(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z) : Eigen::Vector3f(0.0f, 0.0f, 0.0f)});
    }

    // 索引数据
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    if (vertices.empty())
    {
        std::cerr << "WARNING::Mesh::Loaded mesh has no vertices. Skipping setup." << std::endl;
        return; // 不进行 setupMesh
    }

    if (indices.empty())
    {
        std::cerr << "WARNING::Mesh::Loaded mesh has no indices. Skipping setup." << std::endl;
        return; // 不进行 setupMesh
    }

    setupMesh();
    updateModelMatrix(); // 初始化模型矩阵
}

Mesh::~Mesh() {
}

void Mesh::setupMesh() {

    VAO_ = std::make_unique<VertexArray>(); // 创建 VertexArray 对象
    VBO_ = std::make_unique<VertexBuffer>(vertices.data(), vertices.size() * sizeof(Vertex), GL_STATIC_DRAW); // 创建 VertexBuffer 对象
    EBO_ = std::make_unique<IndexBuffer>(indices.data(), indices.size(), GL_STATIC_DRAW); // 创建 IndexBuffer 对象

    VAO_->bind(); // 绑定 VAO
    VAO_->setIndexBuffer(*EBO_); // 设置索引缓冲区

    VAO_->setAttribute(0, *VBO_, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position), sizeof(Vertex), 0); // 位置
    VAO_->setAttribute(1, *VBO_, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal), sizeof(Vertex), 0);   // 法线
    VAO_->setAttribute(2, *VBO_, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texCoords), sizeof(Vertex), 0); // 纹理坐标
    VAO_->setAttribute(3, *VBO_, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, tangent), sizeof(Vertex), 0);   // 切线
    VAO_->setAttribute(4, *VBO_, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, bitangent), sizeof(Vertex), 0); // 副切线

    VAO_->unbind(); // 解绑 VAO
}

void Mesh::render(Shader& shader) const {
    shader.use();

    // 设置模型矩阵 Uniform
    shader.setMat4("model", modelMatrix_);

    // 绑定材质的纹理和 Uniforms
    if (material_) {
        material_->bindTextures(shader);
        material_->setUniforms(shader); // 假设 Material 类有这个方法
    }

    VAO_->bind();
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    VAO_->unbind();
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