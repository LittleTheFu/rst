// mesh/MeshGLData.h
#ifndef MESH_GL_DATA_H
#define MESH_GL_DATA_H

#include <glad/glad.h>
#include <vector>
#include <string> // for name, though in ECS it might be an entity property
#include <Eigen/Dense> // for AABB, if AABB is stored per mesh GL data
#include "Vertex.h" // 确保你有这个头文件，它定义了Vertex结构
#include "vertexArray.h" // 你的 VertexArray 类
#include "indexBuffer.h" // 你的 IndexBuffer 类
#include "vertexBuffer.h" // 你的 VertexBuffer 类
#include "BoundingVolume.h" // 你的 AABB 定义

// MeshGLData: 封装一个网格的OpenGL资源和基本绘制信息
class MeshGLData {
public:
    // 构造函数，直接生成并上传VAO/VBO/EBO数据
    MeshGLData(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    ~MeshGLData(); // 析构函数，负责释放OpenGL资源

    // 绑定VAO以便绘制
    void Bind() const;
    // 解绑VAO
    void Unbind() const;

    // 获取绘制所需的索引数量
    unsigned int getNumIndices() const { return static_cast<unsigned int>(indices_.size()); }
    // 获取网格的局部AABB
    const AABB& getLocalAABB() const { return localAABB_; }

private:
    std::vector<unsigned int> indices_; // 只需要保留索引数量，实际数据可以不保留
                                        // 但为了 getNumIndices() 方便，可以保留大小
    std::unique_ptr<VertexArray> VAO_;
    std::unique_ptr<VertexBuffer> VBO_;
    std::unique_ptr<IndexBuffer> EBO_;

    AABB localAABB_; // 局部边界盒

    // 内部设置OpenGL缓冲区的方法
    void setupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);

    // 禁止拷贝构造和赋值操作，因为OpenGL句柄不能简单拷贝
    MeshGLData(const MeshGLData&) = delete;
    MeshGLData& operator=(const MeshGLData&) = delete;
};

#endif // MESH_GL_DATA_H