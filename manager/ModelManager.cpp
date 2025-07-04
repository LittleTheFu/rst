#include "ModelManager.h"
#include <stdexcept>
#include <filesystem> // C++17 for path manipulation
#include <algorithm>  // For std::replace

// 如果你的 Eigen::Matrix4f 初始化需要，请确保在某个地方定义了 Eigen::Matrix4f::Identity()
// 或者使用 Eigen::Matrix4f::Identity() 在 Eigen::Dense 中定义

// Static member initialization
ModelManager& ModelManager::getInstance() {
    static ModelManager instance;
    return instance;
}

ModelManager::ModelManager() {
    std::cout << "ModelManager: Initialized." << std::endl;
}

ModelManager::~ModelManager() {
    clearAllModels();
    std::cout << "ModelManager: Shut down. All cached ModelResources released." << std::endl;
}

// --- 公共加载/获取接口实现 ---

std::shared_ptr<ModelResource> ModelManager::loadModel(const std::string& filePath, const std::string& modelName) {
    // 决定模型的唯一名称 (ID)
    std::string effectiveModelName = modelName;
    if (effectiveModelName.empty()) {
        // 从文件路径中提取文件名作为默认名称
        std::filesystem::path p(filePath);
        effectiveModelName = p.stem().string(); // 文件名（无扩展名）
    }

    // 1. 检查缓存
    auto it = m_loadedModels.find(effectiveModelName);
    if (it != m_loadedModels.end()) {
        std::cout << "ModelManager: Reusing cached ModelResource: " << effectiveModelName << std::endl;
        return it->second;
    }

    // 2. 模型不在缓存中，开始加载
    Assimp::Importer importer;
    // 使用 Assimp 导入器加载模型，并应用一些后处理选项
    // aiProcess_Triangulate: 将所有非三角形面转换为三角形
    // aiProcess_FlipUVs: 翻转纹理的 Y 轴（如果需要，取决于你的纹理加载器）
    // aiProcess_GenNormals: 如果模型没有法线，则生成法线
    // aiProcess_CalcTangentSpace: 计算切线和副切线，用于法线贴图
    // aiProcess_JoinIdenticalVertices: 连接相同的顶点
    // aiProcess_GenSmoothNormals: 为所有网格生成平滑法线
    // aiProcess_LimitBoneWeights: 限制每个顶点骨骼权重为4个
    // aiProcess_ValidateDataStructure: 验证导入场景的数据结构
    const aiScene* scene = importer.ReadFile(filePath, 
                                            aiProcess_Triangulate | 
                                            aiProcess_FlipUVs | 
                                            aiProcess_GenNormals | 
                                            aiProcess_CalcTangentSpace |
                                            aiProcess_JoinIdenticalVertices |
                                            aiProcess_LimitBoneWeights |
                                            aiProcess_ValidateDataStructure);

    // 检查导入是否成功
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ModelManager ERROR: Failed to load model from '" << filePath << "': " << importer.GetErrorString() << std::endl;
        return nullptr;
    }

    std::shared_ptr<ModelResource> newModel = std::make_shared<ModelResource>(effectiveModelName);
    std::string modelDirectory = getDirectoryPath(filePath);

    std::cout << "ModelManager: Loading model '" << effectiveModelName << "' from: " << filePath << std::endl;

    // 递归处理 Assimp 场景的根节点
    try {
        processNode(scene->mRootNode, scene, newModel, modelDirectory, Eigen::Matrix4f::Identity());
    } catch (const std::exception& e) {
        std::cerr << "ModelManager ERROR: Exception during model processing for '" << effectiveModelName << "': " << e.what() << std::endl;
        return nullptr;
    }

    // 3. 将新加载的模型添加到缓存
    m_loadedModels[effectiveModelName] = newModel;
    std::cout << "ModelManager: Successfully loaded and cached ModelResource: " << effectiveModelName << std::endl;
    return newModel;
}

std::shared_ptr<ModelResource> ModelManager::getModel(const std::string& modelName) const {
    auto it = m_loadedModels.find(modelName);
    if (it != m_loadedModels.end()) {
        return it->second;
    }
    return nullptr; // 未找到
}

void ModelManager::unloadModel(const std::string& modelName) {
    if (m_loadedModels.erase(modelName) > 0) {
        std::cout << "ModelManager: Unloaded ModelResource: " << modelName << std::endl;
    } else {
        std::cout << "ModelManager: ModelResource not found in cache for unload: " << modelName << std::endl;
    }
}

void ModelManager::clearAllModels() {
    m_loadedModels.clear();
    std::cout << "ModelManager: All ModelResources cleared from cache." << std::endl;
}

// --- Assimp 辅助函数实现 ---

void ModelManager::processNode(aiNode* node, const aiScene* scene, 
                               std::shared_ptr<ModelResource> modelResource, 
                               const std::string& modelDirectory,
                               const Eigen::Matrix4f& parentTransform) {
    // 获取当前节点的局部变换
    Eigen::Matrix4f currentTransform = parentTransform * convertMatrixToEigen(node->mTransformation);

    // 处理当前节点的所有网格
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]]; // 获取 Assimp 网格
        processMesh(mesh, scene, modelResource, modelDirectory, currentTransform);
    }

    // 递归处理子节点
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, modelResource, modelDirectory, currentTransform);
    }
}

bool ModelManager::processMesh(aiMesh* mesh, const aiScene* scene, 
                               std::shared_ptr<ModelResource> modelResource, 
                               const std::string& modelDirectory,
                               const Eigen::Matrix4f& meshTransform) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // 提取顶点数据
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        // 位置
        vertex.position = Eigen::Vector3f(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        // 法线
        if (mesh->HasNormals()) {
            vertex.normal = Eigen::Vector3f(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        } else {
            vertex.normal = Eigen::Vector3f(0.0f, 1.0f, 0.0f); // 默认向上
        }
        // 纹理坐标
        if (mesh->mTextureCoords[0]) { // 模型可能有多组纹理坐标，我们只用第一组
            vertex.texCoords = Eigen::Vector2f(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        } else {
            vertex.texCoords = Eigen::Vector2f(0.0f, 0.0f);
        }
        // 切线
        if (mesh->HasTangentsAndBitangents()) {
            vertex.tangent = Eigen::Vector3f(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
            vertex.bitangent = Eigen::Vector3f(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
        } else {
            vertex.tangent = Eigen::Vector3f::Zero();
            vertex.bitangent = Eigen::Vector3f::Zero();
        }

        // 骨骼权重和 ID (Assimp 骨骼处理通常需要单独的逻辑)
        // 这里的 Vertex 结构体已经包含了 boneIDs 和 boneWeights
        // 如果 Assimp 导入器设置了 aiProcess_LimitBoneWeights，这里可以安全地假定最多4个
        // Assimp 的骨骼信息通常在 aiMesh 的 mBones 数组中。
        // 对于每个顶点，需要遍历其所有骨骼权重并填充 Vertex::boneIDs 和 Vertex::boneWeights
        // 这是一个简化的示例，实际骨骼导入可能更复杂
        for (unsigned int boneIdx = 0; boneIdx < 4; ++boneIdx) { // 初始化为0
            vertex.boneIDs[boneIdx] = 0;
            vertex.boneWeights[boneIdx] = 0.0f;
        }

        // 骨骼处理的简化示例：
        // 真正的骨骼数据解析需要遍历 mesh->mBones，
        // 然后对每个 bone 遍历其 mWeights，找到对应的顶点，并填充 boneIDs 和 boneWeights。
        // 这通常是一个复杂的过程，涉及骨骼索引映射和权重归一化。
        // 在此处为简洁起见，暂时省略详细的骨骼权重分配逻辑。
        // 如果你需要骨骼动画，这部分将需要进一步的实现。
        //
        // 示例伪代码（需要更详细的实现）：
        // for (unsigned int bone_idx = 0; bone_idx < mesh->mNumBones; ++bone_idx) {
        //     aiBone* bone = mesh->mBones[bone_idx];
        //     // 将 bone->mName 映射到你的内部骨骼 ID
        //     // for (unsigned int weight_idx = 0; weight_idx < bone->mNumWeights; ++weight_idx) {
        //     //     const aiVertexWeight& vw = bone->mWeights[weight_idx];
        //     //     // if vw.mVertexId == i (current vertex)
        //     //     // add vw.mWeight to vertex.boneWeights, bone->mName (or mapped ID) to vertex.boneIDs
        //     // }
        // }

        vertices.push_back(vertex);
    }

    // 提取索引数据
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // 使用 MeshManager 加载 MeshGLData
    std::string meshName = mesh->mName.C_Str();
    // 如果 Assimp 网格没有名称，生成一个临时名称
    if (meshName.empty()) {
        static unsigned int s_anonMeshCounter = 0;
        meshName = modelResource->name + "_anon_mesh_" + std::to_string(s_anonMeshCounter++);
    }
    
    unsigned int meshGLDataId = MeshManager::getInstance().loadMeshGLData(vertices, indices, meshName);

    if (meshGLDataId == 0) {
        std::cerr << "ModelManager ERROR: Failed to load MeshGLData for mesh '" << meshName << "' in model '" << modelResource->name << "'" << std::endl;
        return false;
    }

    // 使用 MaterialManager 加载材质
    aiMaterial* aiMat = scene->mMaterials[mesh->mMaterialIndex];
    std::shared_ptr<Material> material = MaterialManager::getInstance().loadMaterial(aiMat, modelDirectory);
    
    if (!material) {
        std::cerr << "ModelManager WARNING: Failed to load material for mesh '" << meshName << "' in model '" << modelResource->name << "'" << std::endl;
        // 如果材质加载失败，可以设置为默认材质或者返回 false
        // 这里为了继续流程，我们假设 materialManager 会返回一个默认材质或者我们在这里创建一个
        // MaterialManager::loadMaterial 应该能处理返回 nullptr 的情况
    }

    // 将网格信息添加到 ModelResource
    ModelMeshInfo meshInfo;
    meshInfo.meshId = meshGLDataId;
    // 使用材质的名称作为 ModelMeshInfo 中的材质 ID
    meshInfo.materialName = material ? material->getID() : "default_material"; 
    meshInfo.localTransform = meshTransform; // 存储局部变换

    modelResource->meshes.push_back(meshInfo);

    return true;
}

// 辅助函数：将 Assimp 的 aiMatrix4x4 转换为 Eigen::Matrix4f
Eigen::Matrix4f ModelManager::convertMatrixToEigen(const aiMatrix4x4& from) {
    Eigen::Matrix4f to;
    // Assimp 矩阵是行主序，Eigen 默认列主序，所以需要转置或逐元素赋值
    // aiMatrix4x4 (row-major) to Eigen::Matrix4f (column-major)
    to << from.a1, from.a2, from.a3, from.a4,
          from.b1, from.b2, from.b3, from.b4,
          from.c1, from.c2, from.c3, from.c4,
          from.d1, from.d2, from.d3, from.d4;
    return to.transpose(); // Assimp to Eigen requires transpose for column-major
}

// 辅助函数：从文件路径中提取目录
std::string ModelManager::getDirectoryPath(const std::string& filePath) {
    std::filesystem::path p(filePath);
    return p.parent_path().string();
}