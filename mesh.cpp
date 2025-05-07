#include "Mesh.h"
#include <iostream>

Mesh::Mesh(const std::string& filePath) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_GenNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    // 假设我们只处理第一个网格
    unsigned int meshIndex = scene->mRootNode->mChildren[0]->mMeshes[0];
    aiMesh* mesh = scene->mMeshes[meshIndex];
    // aiMesh* mesh = scene->mRootNode->mChildren[0]->mMeshes[0]; // 确保这里是 aiMesh*

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

void Mesh::draw(Shader& shader) {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}