#include "Model.h"
#include <iostream>
#include "Texture.h"   // 假设你有Texture类来管理OpenGL纹理
#include "Texture2D.h" // 如果你使用Texture2D::loadFromFile
#include <textureManager.h>
// #include "stb_image.h" // 用于加载图片，如果你的Texture类使用它

// 注意：如果你的 Eigen/Geometry 头文件里没有这几个，可能需要添加
// #include <Eigen/Dense>
// #include <Eigen/Geometry>

Model::Model(const std::string& path)
    : position_(0.0f, 0.0f, 0.0f),
      rotation_(Eigen::Quaternionf::Identity()),
      scale_(1.0f, 1.0f, 1.0f)
{
    loadModel(path);
    size_t lastSlash = path.find_last_of("/\\");
    name_ = (lastSlash == std::string::npos) ? path : path.substr(lastSlash + 1);
    size_t dotPos = name_.find_last_of('.');
    if (dotPos != std::string::npos) {
        name_ = name_.substr(0, dotPos);
    }
    updateModelMatrix();
}

Model::~Model() {
    // unique_ptr 会自动管理Animator的生命周期
}

// --- IRenderable 接口实现 ---
void Model::render(Shader& shader) const {
    shader.setMat4("model", modelMatrix_); 

    // 新增：传递骨骼变换矩阵到Shader
    if (animator_ && animator_->GetCurrentAnimation()) { // 仅当有Animator且正在播放动画时才传递
    // if(false)   { //for debug
        shader.setBool("isAnimated", true); // 通知Shader这是个动画模型
        const auto& boneMatrices = animator_->GetFinalBoneMatrices();
        for (int i = 0; i < boneMatrices.size(); ++i) {
            shader.setMat4("finalBoneMatrices[" + std::to_string(i) + "]", boneMatrices[i]);
        // shader.setMat4("finalBoneMatrices[" + std::to_string(i) + "]", Eigen::Matrix4f::Identity());
        }
    } else {
        shader.setBool("isAnimated", false); // 通知Shader这是个静态模型
    }

    for (const auto& mesh : meshes_) {
        mesh->render(shader);
    }
}

// 新增：更新动画的方法
void Model::update(float deltaTime) {
    if (animator_) {
        animator_->UpdateAnimation(deltaTime);
    }
}

// 新增：播放动画的方法
void Model::playAnimation(const std::string& animationName) {
    if (animator_) {
        animator_->PlayAnimation(animationName);
    }
}

// --- ITransformable 接口实现 ---
Eigen::Matrix4f Model::getModelMatrix() const {
    return modelMatrix_;
}

void Model::setModelMatrix(const Eigen::Matrix4f& modelMatrix) {
    modelMatrix_ = modelMatrix;
    // 如果直接设置了矩阵，通常这意味着我们不再通过 position/rotation/scale 来管理，
    // 或者需要从矩阵中反推这些值，这里简化处理，只更新矩阵。
    // 如果需要从矩阵反推位移旋转缩放，会更复杂。
}

Eigen::Vector3f Model::getPosition() const {
    return position_;
}

void Model::Model::setPosition(const Eigen::Vector3f& position) {
    position_ = position;
    updateModelMatrix(); // 位置改变，更新模型矩阵
}

Eigen::Quaternionf Model::getRotation() const {
    return rotation_;
}

void Model::setRotation(const Eigen::Quaternionf& rotation) {
    rotation_ = rotation;
    rotation_.normalize(); // 归一化四元数，避免浮点误差累积
    updateModelMatrix(); // 旋转改变，更新模型矩阵
}

Eigen::Vector3f Model::getScale() const {
    return scale_;
}

void Model::setScale(const Eigen::Vector3f& scale) {
    scale_ = scale;
    updateModelMatrix(); // 缩放改变，更新模型矩阵
}

Eigen::Vector3f Model::getLocalBoundingBoxHalfExtents() const
{
    Eigen::Vector3f localExtents = getWorldAABB()->GetSize();

    //hot fix,why?it's now only be used by physics system.
    float minValue = 0.1f;
    localExtents.x() = std::max(localExtents.x(), minValue);
    localExtents.y() = std::max(localExtents.y(), minValue);
    localExtents.z() = std::max(localExtents.z(), minValue);
   
    return localExtents / 2.0f;
}

std::unique_ptr<AABB> Model::getWorldAABB() const {
    if (meshes_.empty()) {
        return nullptr; // 如果没有 Mesh，则没有 AABB
    }

    // 初始化一个空的 AABB，它会通过 Extend() 逐渐扩展
    AABB combinedAABB;
    // 使用极值初始化，确保第一次 Extend 正确工作
    combinedAABB.min_ = Eigen::Vector3f(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    combinedAABB.max_ = Eigen::Vector3f(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());

    // 遍历所有子 Mesh，并计算它们在世界空间中的 AABB，然后合并
    for (const auto& mesh : meshes_) {
        // 调用 Mesh 的 getWorldAABB 方法，传入 Model 自身的 modelMatrix_
        std::unique_ptr<AABB> meshWorldAABB = mesh->getWorldAABB(modelMatrix_);
        if (meshWorldAABB) {
            combinedAABB.Extend(*meshWorldAABB);
        }
    }
    return std::make_unique<AABB>(combinedAABB);
}

// --- Model 特有的方法 ---
void Model::setMaterial(std::shared_ptr<Material> material) {
    for (auto& mesh : meshes_) {
        mesh->setMaterial(material);
    }
}

std::unique_ptr<Mesh> Model::takeMesh(size_t index) {
    if (index >= meshes_.size()) {
        return nullptr;
    }
    // 使用 std::move 将 unique_ptr 的所有权转移出去，并从 vector 中移除该元素
    std::unique_ptr<Mesh> takenMesh = std::move(meshes_[index]);
    meshes_.erase(meshes_.begin() + index); // 移除元素以保持 vector 紧凑
    return takenMesh;
}

// --- 私有辅助函数 ---
void Model::updateModelMatrix() {
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
    // 顺序很重要：先应用局部缩放和旋转，最后是世界空间平移
    modelMatrix_ = translationMatrix * rotationMatrix * scaleMatrix;
}


void Model::loadModel(const std::string& path) {
    Assimp::Importer importer;
    // 添加 aiProcess_LimitBoneWeights 处理，确保每个顶点最多4个骨骼影响
    // aiProcess_PopulateArmatureData 对某些格式（如glTF）有用，但FBX通常不需要显式此项
    const aiScene* scene = importer.ReadFile(path, 
        aiProcess_Triangulate | 
        aiProcess_GenNormals | 
        aiProcess_CalcTangentSpace |
        aiProcess_FlipUVs |
        aiProcess_JoinIdenticalVertices | 
        aiProcess_GenBoundingBoxes |
        aiProcess_LimitBoneWeights | // 限制每个顶点影响的骨骼数量 (Assimp默认4)
        aiProcess_ValidateDataStructure); 

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::MODEL::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    directory_ = path.substr(0, path.find_last_of("/\\") + 1);
    size_t lastSlash = path.find_last_of("/\\");
    name_ = (lastSlash == std::string::npos) ? path : path.substr(lastSlash + 1);
    size_t dotPos = name_.find_last_of('.');
    if (dotPos != std::string::npos) {
        name_ = name_.substr(0, dotPos);
    }

    // 1. 在处理节点前，先扫描所有网格以收集骨骼信息
    // 这样Animator在构建骨骼层次时就能知道所有骨骼的ID和它们的名称
    int boneCounter = 0;
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[i];
        if (mesh->HasBones()) {
            for (unsigned int j = 0; j < mesh->mNumBones; ++j) {
                aiBone* bone = mesh->mBones[j];
                std::string boneName = bone->mName.C_Str();
                if (boneInfoMap_.find(boneName) == boneInfoMap_.end()) {
                    // 如果骨骼是第一次遇到，分配一个ID
                    boneInfoMap_[boneName] = boneCounter++;
                    // 骨骼的 inverseBindPoseTransform 会在 Animator::BuildBoneHierarchy 中设置
                    // 或者你可以在这里创建一个结构体来存储，然后传递给 Animator
                    // 这里我们只传递 boneInfoMap_ (名称->ID)
                    // Animator 会自己从 aiMesh 遍历来找到 offsetMatrix
                }
            }
        }
    }
    
    // 2. 初始化 Animator
    animator_ = std::make_unique<Animator>();
    animator_->Init(scene, boneInfoMap_); // 传递 Assimp 场景和收集到的骨骼信息

    // 3. 递归处理 Assimp 场景的根节点及其子节点
    // 在这里，我们只处理网格数据，骨骼层次和动画数据已由 Animator 处理
    processNode(scene->mRootNode, scene, Eigen::Matrix4f::Identity()); // 根节点的父变换是单位矩阵
}


// 修改 processNode 以传递 Assimp 节点自身的变换矩阵
void Model::processNode(aiNode* node, const aiScene* scene, const Eigen::Matrix4f& parentTransform) {
    Eigen::Matrix4f nodeTransform = AssimpUtils::ConvertAssimpMat4ToEigen(node->mTransformation);
    Eigen::Matrix4f currentGlobalTransform = parentTransform * nodeTransform;
    nodeTransformMap_[node->mName.C_Str()] = currentGlobalTransform; // 存储节点的全局变换

    // 处理当前节点的所有 Mesh
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes_.push_back(processMesh(mesh, scene));
    }
    // 递归处理子节点
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene, currentGlobalTransform); // 传递当前节点的全局变换作为子节点的父变换
    }
}

std::unique_ptr<Mesh> Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::shared_ptr<Material> material = nullptr;

    bool hasNormals = mesh->HasNormals();
    bool hasTextureCoords = mesh->HasTextureCoords(0); 
    bool hasTangents = mesh->HasTangentsAndBitangents();

    // 预设骨骼ID和权重为0
    Eigen::Vector4i tempBoneIDs = Eigen::Vector4i::Zero();
    Eigen::Vector4f tempBoneWeights = Eigen::Vector4f::Zero();

    // 临时存储每个顶点的骨骼影响信息
    std::vector<std::vector<std::pair<int, float>>> boneInfluences(mesh->mNumVertices);

    // 1. 处理骨骼权重和ID (在遍历顶点之前)
    if (mesh->HasBones()) {
        for (unsigned int i = 0; i < mesh->mNumBones; ++i) {
            aiBone* bone = mesh->mBones[i];
            std::string boneName = bone->mName.C_Str();
            int boneID = -1;
            
            // 从预先构建的 boneInfoMap 中获取骨骼ID
            auto it = boneInfoMap_.find(boneName);
            if (it != boneInfoMap_.end()) {
                boneID = it->second;
            } else {
                std::cerr << "WARNING::MODEL::processMesh: Bone '" << boneName << "' found in mesh but not in global bone map! Assigning dummy ID." << std::endl;
                // 这不应该发生如果 boneInfoMap 在 loadModel 中正确收集了所有骨骼
                // 暂时给一个无效ID，实际可能导致问题
                boneID = MAX_BONES; 
            }

            for (unsigned int j = 0; j < bone->mNumWeights; ++j) {
                aiVertexWeight weight = bone->mWeights[j];
                unsigned int vertexId = weight.mVertexId;
                float w = weight.mWeight;
                if (vertexId < mesh->mNumVertices) {
                    boneInfluences[vertexId].push_back({boneID, w});
                }
            }
        }
    }

    // 2. 处理顶点数据
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex;
        vertex.position = AssimpUtils::ConvertAssimpVec3ToEigen(mesh->mVertices[i]);
        
        if (hasNormals) {
            vertex.normal = AssimpUtils::ConvertAssimpVec3ToEigen(mesh->mNormals[i]);
        } else {
            vertex.normal = Eigen::Vector3f::Zero();
        }
        
        if (hasTextureCoords) {
            vertex.texCoords = Eigen::Vector2f(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        } else {
            vertex.texCoords = Eigen::Vector2f(0.0f, 0.0f);
        }
        
        if (hasTangents) {
            vertex.tangent = AssimpUtils::ConvertAssimpVec3ToEigen(mesh->mTangents[i]);
            vertex.bitangent = AssimpUtils::ConvertAssimpVec3ToEigen(mesh->mBitangents[i]);
        } else {
            vertex.tangent = Eigen::Vector3f::Zero();
            vertex.bitangent = Eigen::Vector3f::Zero();
        }

        // 填充顶点的 boneIDs 和 boneWeights
        // 限制最多4个骨骼影响，按权重降序排列并归一化
        if (!boneInfluences[i].empty()) {
            // 按权重降序排序
            std::sort(boneInfluences[i].begin(), boneInfluences[i].end(), 
                      [](const auto& a, const auto& b){ return a.second > b.second; });

            float totalWeight = 0.0f;
            for (int k = 0; k < std::min((int)boneInfluences[i].size(), 4); ++k) {
                vertex.boneIDs[k] = boneInfluences[i][k].first;
                vertex.boneWeights[k] = boneInfluences[i][k].second;
                totalWeight += boneInfluences[i][k].second;
            }
            // 归一化权重，确保和为1
            if (totalWeight > 0.0f) {
                vertex.boneWeights /= totalWeight;
            }
        }
        vertices.push_back(vertex);
    }

    // 3. 处理索引数据 (保持不变)
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // 4. 处理材质 (保持不变)
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* aiMat = scene->mMaterials[mesh->mMaterialIndex];
        material = processMaterial(aiMat);
    }
    if (!material) {
        material = std::make_shared<Material>();
        material->setAlbedoColor(Eigen::Vector3f(0.8f, 0.8f, 0.8f));
    }

    return std::make_unique<Mesh>(mesh->mName.C_Str(), vertices, indices, material);
}


std::shared_ptr<Material> Model::processMaterial(aiMaterial* aiMat) {
    std::shared_ptr<Material> material = std::make_shared<Material>();

    // 获取颜色属性 (PBR 优先，如果模型有的话)
    aiColor3D color;
    float value;

    // PBR 金属度/粗糙度工作流的颜色因子
    // AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR 对应 AlbedoColor
    // if (aiMat->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, color) == AI_SUCCESS) {
    //     material->setAlbedoColor(AssimpUtils::ConvertAssimpColor3DToEigen(color));
    // } else if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) { // 回退到传统漫反射颜色
    //     material->setAlbedoColor(AssimpUtils::ConvertAssimpColor3DToEigen(color));
    // } else {
    //     material->setAlbedoColor(Eigen::Vector3f(1.0f, 1.0f, 1.0f)); // 默认白色
    // }

    // 金属度因子
    // if (aiMat->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, value) == AI_SUCCESS) {
    //     material->setMetallic(value);
    // } else {
    //     material->setMetallic(0.0f); // 默认非金属
    // }

    // 粗糙度因子
    // if (aiMat->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, value) == AI_SUCCESS) {
    //     material->setRoughness(value);
    // } else if (aiMat->Get(AI_MATKEY_SHININESS, value) == AI_SUCCESS) { // 可以从光泽度推断粗糙度 (非线性关系)
    //     material->setRoughness(1.0f - value / 256.0f); // 这是一个简单的转换，实际 PBR 转换更复杂
    // } else {
    //     material->setRoughness(0.5f); // 默认中等粗糙度
    // }

    // 自发光颜色
    // if (aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS) {
    //     material->setEmissiveColor(AssimpUtils::ConvertAssimpColor3DToEigen(color));
    // } else {
    //     material->setEmissiveColor(Eigen::Vector3f(0.0f, 0.0f, 0.0f));
    // }

    // 加载纹理
    // 这里我们假设你的 Texture 类有一个静态方法 loadFromFile(path)，返回 shared_ptr<Texture>
    // 并且 TextureManager (如果你有) 会管理纹理的加载和存储
    
    // Albedo / Base Color 纹理
    std::vector<std::shared_ptr<Texture>> albedoMaps = loadMaterialTextures(aiMat, aiTextureType_BASE_COLOR, "texture_basecolor");
    if( albedoMaps.empty() ) { // 如果没有 BASE_COLOR，尝试 DIFFUSE
        albedoMaps = loadMaterialTextures(aiMat, aiTextureType_DIFFUSE, "texture_diffuse");
    }
    if( !albedoMaps.empty() ) {
        material->setAlbedoMap(albedoMaps.at(0));
    }

    // 法线纹理
    std::vector<std::shared_ptr<Texture>> normalMaps = loadMaterialTextures(aiMat, aiTextureType_NORMALS, "texture_normal");
    if( !normalMaps.empty() ) {
        material->setNormalMap(normalMaps.at(0));
    }

    // 金属度纹理 (METALNESS)
    std::vector<std::shared_ptr<Texture>> metallicMaps = loadMaterialTextures(aiMat, aiTextureType_METALNESS, "texture_metallic");
    if( !metallicMaps.empty() ) {
        material->setMetallicMap(metallicMaps.at(0));
    }

    // 粗糙度纹理 (DIFFUSE_ROUGHNESS)
    std::vector<std::shared_ptr<Texture>> roughnessMaps = loadMaterialTextures(aiMat, aiTextureType_DIFFUSE_ROUGHNESS, "texture_roughness");
    if( !roughnessMaps.empty() ) {
        material->setRoughnessMap(roughnessMaps.at(0));
    }

    // 环境光遮蔽纹理 (AMBIENT_OCCLUSION)
    std::vector<std::shared_ptr<Texture>> aoMaps = loadMaterialTextures(aiMat, aiTextureType_AMBIENT_OCCLUSION, "texture_ao");
    if( !aoMaps.empty() ) {
        material->setAmbientOcclusionMap(aoMaps.at(0));
    }

    // 自发光纹理 (EMISSIVE)
    // std::vector<std::shared_ptr<Texture>> emissiveMaps = loadMaterialTextures(aiMat, aiTextureType_EMISSIVE, "texture_emissive");
    // if( !emissiveMaps.empty() ) {
    //     material->setEmissiveMap(emissiveMaps.at(0));
    // }

    return material;
}

std::vector<std::shared_ptr<Texture>> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName) {
    std::vector<std::shared_ptr<Texture>> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
        aiString str;
        mat->GetTexture(type, i, &str);
        
        std::string filename = std::string(str.C_Str());
        // 纹理路径可能是相对的，也可能是绝对的
        // 如果是相对路径，需要与模型目录拼接
        std::string fullPath;
        if (filename.empty()) continue; // 路径为空则跳过

        // 简单的路径判断，如果文件名包含路径分隔符，可能就是相对或绝对路径
        // 更健壮的方案是检查文件是否存在，或使用 Assimp 的 asset manager
        if (filename.find('/') != std::string::npos || filename.find('\\') != std::string::npos) {
            fullPath = directory_ + filename; // 假设 Assimp 返回的可能是相对于模型目录的相对路径
        } else {
            fullPath = directory_ + filename;
        }

        // 统一化路径分隔符为 '/'
        for (char &c : fullPath) {
            if (c == '\\') {
                c = '/';
            }
        }
        
        // 尝试加载纹理
        try {
            // 这里假设 Texture2D::loadFromFile 能够处理并缓存纹理
            // 在实际项目中，强烈建议使用一个 TextureManager 来管理纹理的加载和生命周期
            // 避免重复加载相同的纹理
            std::shared_ptr<Texture> newTexture = TextureManager::getInstance().loadTexture2D(fullPath); // 假设 Texture2D::loadFromFile 返回 shared_ptr<Texture>
            if (newTexture) {
                textures.push_back(newTexture);
            }
        } catch (const std::exception& e) {
            std::cerr << "WARNING::MODEL::Failed to load texture: " << fullPath << " - " << e.what() << std::endl;
        }
    }
    return textures;
}