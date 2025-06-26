#include "Mesh.h"
#include <iostream>
#include <Eigen/Geometry>
#include <assert.h>
#include <limits> // For std::numeric_limits

// 辅助函数：将 Assimp 的 aiVector3D 转换为 Eigen::Vector3f
Eigen::Vector3f Mesh::ConvertAssimpVec3ToEigen(const aiVector3D& vec) {
    return Eigen::Vector3f(vec.x, vec.y, vec.z);
}

Mesh::Mesh(const std::string &name, const std::string &filePath, std::shared_ptr<Material> material) : name_(name),
                                                                                 position_(Eigen::Vector3f::Zero()),
                                                                                 rotation_(Eigen::Quaternionf::Identity()),
                                                                                 scale_(Eigen::Vector3f::Ones()),
                                                                                 modelMatrix_(Eigen::Matrix4f::Identity()),
                                                                                 material_(material), // 初始化 Material
                                                                                 localAABB_() // 初始化 AABB
{
    Assimp::Importer importer;
    // 注意：aiProcess_GenBoundingBoxes 对于这种方法来说不是严格必需的
    // 因为我们自己遍历顶点来构建 AABB。但保留它也无害。
    const aiScene *scene = importer.ReadFile(filePath,
                                             aiProcess_Triangulate |
                                             aiProcess_GenNormals |
                                             aiProcess_CalcTangentSpace |
                                             aiProcess_FlipUVs |
                                             aiProcess_GenBoundingBoxes); // 保持这个标志，如果以后想用mesh->mAABB也方便

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    // --- 计算整个模型的局部 AABB ---
    // AABB 默认构造函数已经将 min 初始化为 max_float, max 初始化为 min_float
    // 所以可以直接调用 extend

    for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
    {
        aiMesh *mesh = scene->mMeshes[i];

        bool hasTangents = mesh->HasTangentsAndBitangents();
        assert(hasTangents); // 确保你的模型有切线，否则这里会断言失败

        bool hasTextureCoords = mesh->HasTextureCoords(0);
        assert(hasTextureCoords); // 确保你的模型有纹理坐标，否则这里会断言失败

        bool hasNormals = mesh->HasNormals();
        assert(hasNormals); // 确保你的模型有法线，否则这里会断言失败

        // 遍历当前 aiMesh 的所有顶点，并用它们扩展整个 Mesh 的 localAABB_
        for (unsigned int j = 0; j < mesh->mNumVertices; j++)
        {
            Eigen::Vector3f position = ConvertAssimpVec3ToEigen(mesh->mVertices[j]);
            
            vertices.push_back({
                position,
                hasNormals ? ConvertAssimpVec3ToEigen(mesh->mNormals[j]) : Eigen::Vector3f(0.0f, 0.0f, 0.0f),
                hasTextureCoords ? Eigen::Vector2f(mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y) : Eigen::Vector2f(0.0f, 0.0f),
                hasTangents ? ConvertAssimpVec3ToEigen(mesh->mTangents[j]) : Eigen::Vector3f(0.0f, 0.0f, 0.0f),
                hasTangents ? ConvertAssimpVec3ToEigen(mesh->mBitangents[j]) : Eigen::Vector3f(0.0f, 0.0f, 0.0f)
            });

            // 扩展整个 Mesh 的局部 AABB
            localAABB_.Extend(position);
        }

        // 索引数据
        for (unsigned int j = 0; j < mesh->mNumFaces; j++)
        {
            aiFace face = mesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++)
            {
                indices.push_back(face.mIndices[k]);
            }
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

Mesh::~Mesh()
{
    // unique_ptr 会自动管理内存，无需手动 delete VAO_, VBO_, EBO_
}

void Mesh::setupMesh()
{
    VAO_ = std::make_unique<VertexArray>();
    VBO_ = std::make_unique<VertexBuffer>(vertices.data(), vertices.size() * sizeof(Vertex), GL_STATIC_DRAW);
    EBO_ = std::make_unique<IndexBuffer>(indices.data(), indices.size(), GL_STATIC_DRAW);

    VAO_->bind();
    VAO_->setIndexBuffer(*EBO_);

    // 假设 Vertex 结构体和 setAttribute 的参数顺序不变
    VAO_->setAttribute(0, *VBO_, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position), sizeof(Vertex), 0); // 位置
    VAO_->setAttribute(1, *VBO_, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal), sizeof(Vertex), 0);   // 法线
    VAO_->setAttribute(2, *VBO_, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texCoords), sizeof(Vertex), 0); // 纹理坐标
    VAO_->setAttribute(3, *VBO_, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, tangent), sizeof(Vertex), 0);   // 切线
    VAO_->setAttribute(4, *VBO_, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, bitangent), sizeof(Vertex), 0); // 副切线

    VAO_->unbind();
}

void Mesh::render(Shader &shader) const
{
    shader.use();

    // 设置模型矩阵 Uniform
    shader.setMat4("model", modelMatrix_);

    // 绑定材质的纹理和 Uniforms
    if (material_)
    {
        material_->bindTextures(shader);
        material_->setUniforms(shader);
    }

    VAO_->bind();
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    VAO_->unbind();
}

Eigen::Matrix4f Mesh::getModelMatrix() const
{
    return modelMatrix_;
}

void Mesh::setModelMatrix(const Eigen::Matrix4f &modelMatrix)
{
    modelMatrix_ = modelMatrix;
    // 如果你希望在setModelMatrix时也能更新position, rotation, scale
    // 则需要从modelMatrix_中分解出这些分量。这比直接设置它们复杂。
    // 通常，modelMatrix是根据position, rotation, scale计算出来的，
    // 而不是反过来。所以这里不更新position/rotation/scale是正常的。
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
    Eigen::Affine3f model =
        Eigen::Translation3f(position_) *
        rotation_ * // Quaternionf 可以直接用于 Affine3f
        Eigen::Scaling(scale_);

    modelMatrix_ = model.matrix();
}

// 获取世界空间 AABB 的实现
AABB* Mesh::getWorldAABB() const {
    // 调用 localAABB_ 的 Transform 方法，将局部 AABB 变换到世界空间
    // 并返回新创建的 AABB 对象指针
    return localAABB_.Transform(modelMatrix_);
}