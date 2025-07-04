// mesh/MeshGLData.cpp
#include "MeshGLData.h"
#include <iostream>

MeshGLData::MeshGLData(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
    : indices_(indices), // 仅为获取尺寸，实际数据不必须保留
      localAABB_()
{
    // 计算局部AABB
    if (!vertices.empty()) {
        for (const auto& vert : vertices) {
            localAABB_.Extend(vert.position);
        }
    } else {
        std::cerr << "WARNING::MeshGLData::Constructor: No vertices provided for AABB calculation." << std::endl;
    }

    if (vertices.empty() || indices.empty()) {
        std::cerr << "WARNING::MeshGLData::Constructor: No vertices or indices. Skipping GL setup." << std::endl;
        // 这里不调用 setupMesh，因为没有数据
        VAO_ = nullptr; // 明确设置为nullptr
        VBO_ = nullptr;
        EBO_ = nullptr;
        return;
    }

    setupMesh(vertices, indices);
}

MeshGLData::~MeshGLData() {
    // unique_ptr 析构时会自动调用其管理对象的析构函数，
    // VertexArray, VertexBuffer, IndexBuffer 的析构函数应负责释放OpenGL资源。
    // 无需手动 glDeleteBuffers/VertexArrays。
}

void MeshGLData::setupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    VAO_ = std::make_unique<VertexArray>();
    VBO_ = std::make_unique<VertexBuffer>(vertices.data(), vertices.size() * sizeof(Vertex), GL_STATIC_DRAW);
    EBO_ = std::make_unique<IndexBuffer>(indices.data(), indices.size(), GL_STATIC_DRAW);

    VAO_->bind();
    VAO_->setIndexBuffer(*EBO_); // IndexBuffer 绑定到 VAO

    // 顶点属性设置 (与你的Mesh::setupMesh一致)
    VAO_->setAttribute(0, *VBO_, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position), sizeof(Vertex), 0);
    VAO_->setAttribute(1, *VBO_, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal), sizeof(Vertex), 0);
    VAO_->setAttribute(2, *VBO_, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texCoords), sizeof(Vertex), 0);
    VAO_->setAttribute(3, *VBO_, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, tangent), sizeof(Vertex), 0);
    VAO_->setAttribute(4, *VBO_, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, bitangent), sizeof(Vertex), 0);

    // 骨骼ID (GL_INT) - 注意第六个参数 1 表示这是整数属性
    VAO_->setAttribute(5, *VBO_, 4, GL_INT, GL_FALSE, offsetof(Vertex, boneIDs), sizeof(Vertex), 1, true);
    // 骨骼权重 (GL_FLOAT)
    VAO_->setAttribute(6, *VBO_, 4, GL_FLOAT, GL_FALSE, offsetof(Vertex, boneWeights), sizeof(Vertex), 0);

    VAO_->unbind();
}

void MeshGLData::Bind() const {
    if (VAO_) {
        VAO_->bind();
    } else {
        std::cerr << "WARNING::MeshGLData::Bind: VAO is null, cannot bind." << std::endl;
    }
}

void MeshGLData::Unbind() const {
    if (VAO_) {
        VAO_->unbind();
    } else {
        std::cerr << "WARNING::MeshGLData::Unbind: VAO is null, cannot unbind." << std::endl;
    }
}