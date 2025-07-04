#include "MaterialManager.h"
#include <stdexcept> // For std::runtime_error and std::exception
#include <assimp/pbrmaterial.h> // For AI_MATKEY_BASE_COLOR, AI_MATKEY_ROUGHNESS_FACTOR, etc.
#include <assimp/material.h>    // For aiTextureType_UNKNOWN if needed


// Static member initialization: 确保 MaterialManager 实例只在第一次访问时创建
MaterialManager& MaterialManager::getInstance() {
    static MaterialManager instance;
    return instance;
}

// 构造函数：初始化管理器
MaterialManager::MaterialManager() {
    std::cout << "MaterialManager: Initialized." << std::endl;
}

// 析构函数：在管理器销毁时清除所有缓存的材质
MaterialManager::~MaterialManager() {
    clearAllMaterials();
    std::cout << "MaterialManager: Shut down. All cached materials released." << std::endl;
}

// --- 公共加载/获取接口实现 (基于文件约定) ---

// 加载或获取一个材质 (基于文件约定)
std::shared_ptr<Material> MaterialManager::loadMaterial(
    const std::string& materialID,
    const std::string& directory,
    const std::string& extension,
    bool flipY)
{
    // 1. 检查缓存：如果材质已存在，则直接返回
    auto it = m_loadedMaterials.find(materialID);
    if (it != m_loadedMaterials.end()) {
        std::cout << "MaterialManager: Reusing cached Material: " << materialID << std::endl;
        return it->second;
    }

    // 2. 材质不在缓存中，尝试内部加载
    std::shared_ptr<Material> material = nullptr;
    try {
        // 调用内部辅助函数进行加载
        material = internalLoadMaterial(materialID, directory, extension, flipY);
    } catch (const std::runtime_error& e) {
        std::cerr << "MaterialManager ERROR: Failed to load Material '" << materialID << "' from directory '" << directory << "': " << e.what() << std::endl;
        return nullptr;
    } catch (const std::exception& e) { // 捕获其他可能的异常
        std::cerr << "MaterialManager ERROR: An unexpected error occurred while loading Material '" << materialID << "': " << e.what() << std::endl;
        return nullptr;
    }

    // 3. 如果加载成功，则将其添加到缓存中
    if (material) {
        m_loadedMaterials[materialID] = material;
        std::cout << "MaterialManager: Loaded and cached Material: " << materialID << std::endl;
    } else {
        std::cerr << "MaterialManager ERROR: internalLoadMaterial returned nullptr for: " << materialID << std::endl;
    }
    return material;
}

// --- 公共加载/获取接口实现 (从 Assimp aiMaterial) ---

// 从 Assimp aiMaterial 加载材质
std::shared_ptr<Material> MaterialManager::loadMaterial(aiMaterial* aiMat, const std::string& modelDirectory) {
    if (!aiMat) {
        std::cerr << "MaterialManager ERROR: loadMaterial(aiMaterial*): Null aiMaterial pointer provided." << std::endl;
        return nullptr;
    }

    // Assimp 材质通常有一个名称，可以用作缓存的 key
    aiString name;
    aiMat->Get(AI_MATKEY_NAME, name);
    std::string materialID = name.C_Str();

    // 如果 Assimp 材质没有名称，或者名称为空，生成一个唯一的 ID
    if (materialID.empty()) {
        // 为了简单，这里使用一个组合ID。在生产环境中，可能需要更健壮的UUID或哈希生成方案。
        static unsigned int s_anonymousMaterialCount = 0;
        materialID = "AnonMaterial_" + std::to_string(++s_anonymousMaterialCount);
        std::cout << "MaterialManager: Anonymous Assimp material found, assigning ID: " << materialID << std::endl;
    }

    // 1. 检查缓存
    auto it = m_loadedMaterials.find(materialID);
    if (it != m_loadedMaterials.end()) {
        std::cout << "MaterialManager: Reusing cached Assimp Material: " << materialID << std::endl;
        return it->second;
    }

    // 2. 不在缓存中，尝试内部加载 Assimp 材质
    std::shared_ptr<Material> material = nullptr;
    try {
        material = internalLoadMaterialFromAssimp(aiMat, modelDirectory);
        material->setName(materialID); // 设置材质名称，也作为 Asset ID
    } catch (const std::runtime_error& e) {
        std::cerr << "MaterialManager ERROR: Failed to load Assimp Material '" << materialID << "': " << e.what() << std::endl;
        return nullptr;
    } catch (const std::exception& e) {
        std::cerr << "MaterialManager ERROR: An unexpected error occurred while loading Assimp Material '" << materialID << "': " << e.what() << std::endl;
        return nullptr;
    }

    // 3. 如果加载成功，添加到缓存
    if (material) {
        m_loadedMaterials[materialID] = material;
        std::cout << "MaterialManager: Loaded and cached Assimp Material: " << materialID << std::endl;
    } else {
        std::cerr << "MaterialManager ERROR: internalLoadMaterialFromAssimp returned nullptr for: " << materialID << std::endl;
    }
    return material;
}


// 尝试从缓存中获取一个已加载的材质
std::shared_ptr<Material> MaterialManager::getMaterial(const std::string& materialID) {
    auto it = m_loadedMaterials.find(materialID);
    if (it != m_loadedMaterials.end()) {
        return it->second;
    }
    return nullptr; // 未找到
}

// 卸载指定 ID 的材质
void MaterialManager::unloadMaterial(const std::string& materialID) {
    // std::map::erase 返回被删除元素的数量，如果 > 0 表示成功删除
    if (m_loadedMaterials.erase(materialID) > 0) {
        std::cout << "MaterialManager: Unloaded Material: " << materialID << std::endl;
    } else {
        std::cout << "MaterialManager: Material not found in cache for unload: " << materialID << std::endl;
    }
    // 注意：当 shared_ptr 从 map 中移除时，如果其引用计数变为 0，
    // 材质对象（及其内部持有的纹理 shared_ptr）将自动析构并释放资源。
}

// 清空所有已缓存的材质
void MaterialManager::clearAllMaterials() {
    m_loadedMaterials.clear(); // 清空 map，所有 shared_ptr 将被销毁
    std::cout << "MaterialManager: All materials cleared from cache." << std::endl;
    // shared_ptr 的析构机制将确保相关资源被正确释放。
}

// --- 内部加载辅助函数实现 (基于文件约定) ---

// 这个函数封装了调用 MaterialFactory 来创建材质的逻辑
// 注意: MaterialFactory 在你的 MaterialManager.h 中被 #include，
// 但 MaterialManager.cpp 中没有直接使用 MaterialFactory 的接口。
// 这段逻辑实际上已经集成在了 MaterialManager::internalLoadMaterial 中。
std::shared_ptr<Material> MaterialManager::internalLoadMaterial(
    const std::string& materialID,
    const std::string& directory,
    const std::string& extension,
    bool flipY)
{
    // 这里你的 MaterialManager 内部包含了 Material 的创建逻辑，
    // 如果你有一个 MaterialFactory::createMaterial() 这样的接口，
    // 那么这里应该调用它。目前它直接创建 Material 并加载纹理。
    auto material = std::make_shared<Material>(materialID); // 材质名称作为ID

    auto loadTex = [&](const std::string &type) -> std::shared_ptr<Texture2D>
    {
        std::string path = directory + "/" + type + extension;
        // 对于 albedo (颜色贴图)，通常需要 flipY 和 sRGB
        // 对于法线、粗糙度、金属度、AO等数据贴图，通常不需要 flipY 和 sRGB
        bool currentFlipY = (type == "albedo" || type == "basecolor") ? true : false;
        bool currentSrgb = (type == "albedo" || type == "basecolor") ? true : false;

        // 简单判断是否是 DDS，更严谨的应该检查文件内容
        bool isDDS = (extension == ".dds" || extension == ".DDS");

        auto tex = TextureManager::getInstance().loadTexture2D(path, isDDS, currentFlipY, currentSrgb);
        if (!tex)
            std::cerr << "WARNING::MaterialManager::internalLoadMaterial: Failed to load " << type << " texture at " << path << std::endl;
        return tex;
    };

    material->setAlbedoMap(loadTex("albedo"));
    material->setNormalMap(loadTex("normal"));
    material->setRoughnessMap(loadTex("roughness"));
    material->setMetallicMap(loadTex("metallic"));
    material->setAmbientOcclusionMap(loadTex("ao"));

    return material;
}

// --- 内部加载辅助函数实现 (从 Assimp aiMaterial) ---

std::shared_ptr<Material> MaterialManager::internalLoadMaterialFromAssimp(aiMaterial* aiMat, const std::string& modelDirectory) {
    auto material = std::make_shared<Material>(); // Material 默认构造函数会设置默认名称

    // 1. 处理颜色属性
    aiColor3D color(0.f, 0.f, 0.f);
    ai_real value;

    // A. 尝试获取 PBR 属性 (glTF/PBR 常用)
    if (AI_SUCCESS == aiMat->Get(AI_MATKEY_BASE_COLOR, color)) { // glTF PBR baseColor
        material->setAlbedoColor(Eigen::Vector3f(color.r, color.g, color.b));
    } else if (AI_SUCCESS == aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color)) { // 传统漫反射颜色
        material->setAlbedoColor(Eigen::Vector3f(color.r, color.g, color.b));
    } else {
        material->setAlbedoColor(Eigen::Vector3f(1.0f, 1.0f, 1.0f)); // 默认白色
    }

    if (AI_SUCCESS == aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, value)) {
        material->setRoughnessFactor(value);
    } else {
        // 如果没有显式粗糙度，尝试获取旧版 Assimp 的 shininess 并转换为粗糙度
        // 这是一个粗略的转换，PBR 和传统着色模型不完全兼容
        ai_real shininess;
        if (AI_SUCCESS == aiMat->Get(AI_MATKEY_SHININESS, shininess) && shininess != 0.0f) {
            // shininess (0 to infinity) to roughness (0 to 1)
            // 常见的转换公式：roughness = sqrt(2 / (shininess + 2))
            float r = std::sqrt(2.0f / (shininess + 2.0f)); // 近似转换
            material->setRoughnessFactor(r);
        } else {
             material->setRoughnessFactor(0.5f); // 默认值
        }
    }

    if (AI_SUCCESS == aiMat->Get(AI_MATKEY_METALLIC_FACTOR, value)) {
        material->setMetallicFactor(value);
    } else {
        material->setMetallicFactor(0.0f); // 默认非金属
    }

    // Assimp 没有直接的 AI_MATKEY_AMBIENT_OCCLUSION_FACTOR, AO通常通过纹理提供
    // 如果需要因子，可能要从其他属性推断，或使用默认值
    // material->setAmbientOcclusion(1.0f); // 默认无AO，依赖纹理

    // 2. 处理纹理贴图
    // aiTextureType 包含各种纹理类型，如漫反射、高光、法线、PBR 特性等

    // Albedo / Base Color Map
    material->setAlbedoMap(loadTextureAssimp(aiMat, aiTextureType_BASE_COLOR, modelDirectory)); // glTF Base Color
    if (!material->getAlbedoMap()) { // 如果没有 Base Color，尝试传统漫反射
        material->setAlbedoMap(loadTextureAssimp(aiMat, aiTextureType_DIFFUSE, modelDirectory));
    }
    
    // Normal Map
    material->setNormalMap(loadTextureAssimp(aiMat, aiTextureType_NORMALS, modelDirectory));
    if (!material->getNormalMap()) { // 有些旧格式可能用 Height 作为法线贴图
        material->setNormalMap(loadTextureAssimp(aiMat, aiTextureType_HEIGHT, modelDirectory)); 
    }

    // Roughness Map
    material->setRoughnessMap(loadTextureAssimp(aiMat, aiTextureType_DIFFUSE_ROUGHNESS, modelDirectory));
    // Fallback for combined metallic-roughness (often AI_TEXTURE_TYPE_METALNESS or aiTextureType_UNKNOWN with specific parsing)
    // For simplicity, if not diffuse_roughness, we skip combined maps for now.
    
    // Metallic Map
    material->setMetallicMap(loadTextureAssimp(aiMat, aiTextureType_METALNESS, modelDirectory));

    // Ambient Occlusion Map
    material->setAmbientOcclusionMap(loadTextureAssimp(aiMat, aiTextureType_AMBIENT, modelDirectory));
    if (!material->getAmbientOcclusionMap()) { // Fallback for some common cases where AO is stored as Lightmap
        material->setAmbientOcclusionMap(loadTextureAssimp(aiMat, aiTextureType_LIGHTMAP, modelDirectory));
    }
    
    // Additional common maps you might want to add:
    // material->setEmissiveMap(loadTextureAssimp(aiMat, aiTextureType_EMISSIVE, modelDirectory));
    // material->setOpacityMap(loadTextureAssimp(aiMat, aiTextureType_OPACITY, modelDirectory));


    return material;
}

// Helper function to load texture from Assimp material property
std::shared_ptr<Texture2D> MaterialManager::loadTextureAssimp(aiMaterial* mat, aiTextureType type, const std::string& directory) {
    aiString str;
    // GetTexture() takes the texture type, index (always 0 for our purpose), and a reference to aiString
    if (mat->GetTexture(type, 0, &str) == AI_SUCCESS) {
        std::string filename = str.C_Str();
        std::string fullPath = directory + '/' + filename;

        // Determine if it's a DDS texture based on extension (simple check)
        bool isDDS = (filename.length() >= 4 && filename.substr(filename.length() - 4) == ".dds" ||
                      filename.length() >= 4 && filename.substr(filename.length() - 4) == ".DDS");
        
        // PBR textures (roughness, metallic, AO) are typically non-sRGB and usually don't need Y-flipping
        // For Albedo (diffuse), it's typically sRGB and often needs Y-flipping.
        bool flipY = true; // Default for color textures
        bool srgb = true;  // Default for color textures

        // Adjust flags based on texture type
        if (type == aiTextureType_NORMALS || type == aiTextureType_HEIGHT) {
            flipY = false; // Normal maps are data, not visual, usually no flip
            srgb = false;  // Normal maps are data, not color
        } else if (type == aiTextureType_DIFFUSE_ROUGHNESS || type == aiTextureType_METALNESS || 
                   type == aiTextureType_AMBIENT || type == aiTextureType_LIGHTMAP ||
                   type == aiTextureType_SHININESS || type == aiTextureType_SPECULAR) { // Old specular/shininess maps
            // These are typically data maps, not color maps, so no sRGB and often no flip.
            flipY = false; 
            srgb = false; 
        } else if (type == aiTextureType_EMISSIVE) { // Emissive maps are color, but might not need sRGB depending on usage
            flipY = true;
            srgb = true; // Assume sRGB for color
        } else if (type == aiTextureType_OPACITY) { // Opacity maps are data
            flipY = true; // Transparency maps often follow color texture flipping
            srgb = false;
        }
        // aiTextureType_BASE_COLOR is typically sRGB and needs flipY

        // Call TextureManager to load the texture
        std::shared_ptr<Texture2D> texture = TextureManager::getInstance().loadTexture2D(fullPath, isDDS, flipY, srgb);
        return texture;
    }
    return nullptr; // Texture not found or could not be loaded
}