#include "Model.h"
#include <iostream>
#include "Texture.h" // 假设你有Texture类来管理OpenGL纹理
#include <texture2D.h>
#include "assimpUtils.h"
// #include "stb_image.h" // 用于加载图片，如果你的Texture类使用它


Model::Model(const std::string& path) {
    loadModel(path);
}

void Model::render(Shader& shader) const {
    for (const auto& mesh : meshes_) {
        mesh->render(shader); // 渲染每个子 Mesh
    }
}

void Model::setMaterial(std::shared_ptr<Material> material)
{
    for (auto& mesh : meshes_) {
        mesh->setMaterial(material);
    }
}

void Model::loadModel(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, 
        aiProcess_Triangulate | 
        aiProcess_GenNormals | 
        aiProcess_CalcTangentSpace |
        aiProcess_FlipUVs |
        aiProcess_JoinIdenticalVertices | // 额外处理：合并重复顶点
        aiProcess_GenBoundingBoxes);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::MODEL::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    directory_ = path.substr(0, path.find_last_of('/'));
    if (directory_.empty() && path.find_last_of('\\') != std::string::npos) {
        directory_ = path.substr(0, path.find_last_of('\\'));
    }
    
    // 递归处理 Assimp 场景的根节点
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene) {
    // 处理当前节点的所有 Mesh
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes_.push_back(processMesh(mesh, scene));
    }
    // 递归处理子节点
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene);
    }
}

std::unique_ptr<Mesh> Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::shared_ptr<Material> material = nullptr;

    // 1. 处理顶点数据
    bool hasNormals = mesh->HasNormals();
    bool hasTextureCoords = mesh->HasTextureCoords(0);
    bool hasTangents = mesh->HasTangentsAndBitangents();

    // 可以选择在Mesh的构造函数中加入断言或者在这里提前检查，
    // 以避免在没有对应数据时访问空指针。
    // assert(hasNormals); // 你可以根据需要移除或保留这些断言
    // assert(hasTextureCoords);
    // assert(hasTangents);

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
        vertices.push_back(vertex);
    }

    // 2. 处理索引数据
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // 3. 处理材质
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* aiMat = scene->mMaterials[mesh->mMaterialIndex];
        material = processMaterial(aiMat);
    }
    // 如果没有材质或者处理失败，可以给一个默认材质
    if (!material) {
        material = std::make_shared<Material>(); // 创建一个默认的Material实例
        // material->setDiffuseColor(Eigen::Vector3f(0.8f, 0.8f, 0.8f)); // 设置一个灰白色
        material->setAlbedoColor(Eigen::Vector3f(0.8f, 0.8f, 0.8f));
        // 可以在这里设置默认纹理等
    }

    // 返回一个新创建的 Mesh 对象
    return std::make_unique<Mesh>(mesh->mName.C_Str(), vertices, indices, material);
}


std::shared_ptr<Material> Model::processMaterial(aiMaterial* aiMat) {
    std::shared_ptr<Material> material = std::make_shared<Material>();

    // 获取颜色属性
    aiColor3D color;
    if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
        // material->setDiffuseColor(ConvertAssimpColor3DToEigen(color));
        material->setAlbedoColor(AssimpUtils::ConvertAssimpColor3DToEigen(color));
    }
    // if (aiMat->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
    //     material->setSpecularColor(ConvertAssimpColor3DToEigen(color));
    // }
    // if (aiMat->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS) {
    //     material->setAmbientColor(ConvertAssimpColor3DToEigen(color));
    // }
    // if (aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS) {
    //     material->setEmissiveColor(ConvertAssimpColor3DToEigen(color));
    // }

    // float shininess = 0.0f;
    // if (aiMat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
    //     material->setShininess(shininess);
    // }

    // 加载纹理
    // 这里我们假设你的 Texture 类构造函数接受文件路径
    // 并且 TextureManager (如果你有) 会管理纹理的加载和存储
    // 为了简化，这里直接加载，但在实际项目中建议使用 TextureManager
    
    // 漫反射纹理
    std::vector<std::shared_ptr<Texture>> diffuseMaps = loadMaterialTextures(aiMat, aiTextureType_DIFFUSE, "texture_diffuse");
    if( !diffuseMaps.empty() ) {
        material->setAlbedoMap(diffuseMaps.at(0));
    }
    // material->addTextures(diffuseMaps);

    // 高光纹理
    // std::vector<std::shared_ptr<Texture>> specularMaps = loadMaterialTextures(aiMat, aiTextureType_SPECULAR, "texture_specular");
    // material->addTextures(specularMaps);

    // 法线纹理
    std::vector<std::shared_ptr<Texture>> normalMaps = loadMaterialTextures(aiMat, aiTextureType_NORMALS, "texture_normal");
    if( !normalMaps.empty() ) {
        material->setNormalMap(normalMaps.at(0));
    }

    // material->addTextures(normalMaps);

    // 如果是 PBR 模型，你可能还需要加载 metallic, roughness, ao 纹理
    // 例如：
    std::vector<std::shared_ptr<Texture>> metallicMaps = loadMaterialTextures(aiMat, aiTextureType_METALNESS, "texture_metallic");
    if( !metallicMaps.empty() ) {
        material->setMetallicMap(metallicMaps.at(0));
    }
    // material->addTextures(metallicMaps);

    std::vector<std::shared_ptr<Texture>> roughnessMaps = loadMaterialTextures(aiMat, aiTextureType_DIFFUSE_ROUGHNESS, "texture_roughness"); // 注意PBR粗糙度类型
    if( !roughnessMaps.empty() ) {
        material->setRoughnessMap(roughnessMaps.at(0));
    }
    // material->addTextures(roughnessMaps);

    std::vector<std::shared_ptr<Texture>> aoMaps = loadMaterialTextures(aiMat, aiTextureType_AMBIENT_OCCLUSION, "texture_ao");
    if( !aoMaps.empty() ) {
        material->setAmbientOcclusionMap(aoMaps.at(0));
    }

    // material->addTextures(aoMaps);

    return material;
}

std::vector<std::shared_ptr<Texture>> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName) {
    std::vector<std::shared_ptr<Texture>> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
        aiString str;
        mat->GetTexture(type, i, &str);
        
        // 组合完整路径
        std::string filename = std::string(str.C_Str());
        std::string fullPath = directory_ + "/" + filename;

        // 这里需要你的 Texture 类有一个构造函数能够从文件路径加载纹理
        // 或者你有一个 TextureManager 单例来加载和缓存纹理
        // 为了演示，我们假设 Texture::loadFromFile(fullPath, typeName) 存在
        // 并且返回一个 shared_ptr<Texture>
        
        // 注意：实际项目中，你可能会在这里添加纹理缓存机制
        // 例如：TextureManager::getInstance()->getTexture(fullPath, typeName);
        
        try {
            // 假设 Texture 构造函数接受完整路径和类型名称
            // textures.push_back(std::make_shared<Texture>(fullPath, typeName));
            textures.push_back(Texture2D::loadFromFile(fullPath));
        } catch (const std::exception& e) {
            std::cerr << "WARNING::MODEL::Failed to load texture: " << fullPath << " - " << e.what() << std::endl;
        }
    }
    return textures;
}