// ShaderManager.cpp
#include "ShaderManager.h"
#include "debug_utils.h" // 引入 GL_CHECK_ERROR()

// 单例模式的静态实例定义
ShaderManager& ShaderManager::getInstance() {
    static ShaderManager instance;
    return instance;
}

// 私有构造函数
ShaderManager::ShaderManager() {
    std::cout << "ShaderManager initialized." << std::endl;
}

// 私有析构函数
ShaderManager::~ShaderManager() {
    clearAllShaders();
    std::cout << "ShaderManager destroyed." << std::endl;
}

// 生成着色器唯一 Key
std::string ShaderManager::generateShaderKey(const std::string& vertexPath, const std::string& fragmentPath, const std::vector<std::string>& defines) const {
    std::string key = vertexPath + "|" + fragmentPath;
    if (!defines.empty()) {
        key += "|";
        for (const std::string& define : defines) {
            key += define + ";";
        }
    }
    return key;
}

// 加载或获取着色器程序 (不带宏定义)
std::shared_ptr<Shader> ShaderManager::loadShader(const std::string& vertexPath, const std::string& fragmentPath) {
    return loadShader(vertexPath, fragmentPath, {}); // 调用带宏定义的重载
}

// 加载或获取着色器程序 (带宏定义)
std::shared_ptr<Shader> ShaderManager::loadShader(const std::string& vertexPath, const std::string& fragmentPath, const std::vector<std::string>& defines) {
    std::string key = generateShaderKey(vertexPath, fragmentPath, defines);

    // 检查缓存
    if (m_loadedShaders.count(key)) {
        std::cout << "ShaderManager: Found shader in cache: " << key << std::endl;
        return m_loadedShaders[key];
    }

    std::cout << "ShaderManager: Loading new shader: " << key << std::endl;
    // 创建新的 Shader 对象 (std::make_shared 推荐用于 shared_ptr)
    std::shared_ptr<Shader> newShader = std::make_shared<Shader>();

    // 实际加载着色器 (Shader 类内部会处理文件读取、编译和链接)
    // 注意：当前 Shader::load 不支持宏定义注入。如果需要，Shader::load 签名需修改。
    // 为简化，这里直接调用其 load 方法。如果需要支持 defines，需要修改 Shader::load
    // 或者在 ShaderManager 内部先读取文件，注入宏，再传递给 Shader 的编译函数。
    // 目前的 Shader::load 不支持，所以这里的 defines 仅用于生成 Key，不影响实际编译
    // 假设你的 shader 文件内部已经处理了这些宏，或者你将来会修改 Shader::load
    // 来支持预处理器的宏注入。
    newShader->load(vertexPath, fragmentPath); // <-- 这里是关键，Shader::load 应该处理 defines

    if (!newShader->isValid()) {
        std::cerr << "ShaderManager ERROR: Failed to load shader " << key << std::endl;
        // 即使加载失败，如果 newShader 仍是有效指针，它会在离开作用域时被销毁
        // 但为了明确，我们返回 nullptr
        return nullptr;
    }

    // 存储到缓存
    m_loadedShaders[key] = newShader;
    std::cout << "ShaderManager: Successfully loaded and cached shader: " << key << std::endl;
    return newShader;
}

// 从缓存中获取着色器
std::shared_ptr<Shader> ShaderManager::getShader(const std::string& key) {
    if (m_loadedShaders.count(key)) {
        return m_loadedShaders[key];
    }
    std::cerr << "ShaderManager WARNING: Shader '" << key << "' not found in cache." << std::endl;
    return nullptr;
}

// 卸载着色器
void ShaderManager::unloadShader(const std::string& key) {
    if (m_loadedShaders.count(key)) {
        // shared_ptr 会在引用计数变为0时自动释放资源
        m_loadedShaders.erase(key);
        std::cout << "ShaderManager: Unloaded shader: " << key << std::endl;
    } else {
        std::cerr << "ShaderManager WARNING: Attempted to unload non-existent shader: " << key << std::endl;
    }
}

// 清空所有缓存的着色器
void ShaderManager::clearAllShaders() {
    m_loadedShaders.clear(); // shared_ptr 会自动释放所有引用计数为1的资源
    std::cout << "ShaderManager: All cached shaders cleared." << std::endl;
}