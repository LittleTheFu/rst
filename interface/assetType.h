#ifndef _ASSET_TYPE_H_
#define _ASSET_TYPE_H_

#include <string>   // 用于 std::string


// --- AssetType 枚举定义 ---
/**
 * @brief 定义了所有可被资源管理器识别和管理的资产类型。
 *
 * 使用枚举提供类型安全、更好的性能和清晰的代码意图，
 * 避免了字符串比较可能导致的运行时错误。
 */
enum class AssetType {
    Unknown = 0,        // 未知类型，用于初始化或错误情况
    Texture2D,          // 2D 纹理
    TextureCubeMap,     // 立方体贴图纹理
    Material,           // 材质
    Model,              // 完整的模型（可能包含多个网格和材质引用）
    ModelData,          // 模型的几何数据（如顶点、索引，不含材质）
    Shader,             // 着色器程序
    // TODO: 根据你的项目需求，在这里添加更多资产类型，例如：
    // Audio,             // 音频文件
    // Animation,         // 动画数据
    // SceneNode,         // 场景节点数据
    
    COUNT               // 辅助值，表示枚举成员的总数，总是放在最后
};

/**
 * @brief 将 AssetType 枚举值转换为对应的字符串表示。
 *
 * 主要用于调试、日志输出和 UI 显示，方便人类可读。
 *
 * @param type 要转换的 AssetType 枚举值。
 * @return 资产类型名称的 C 风格字符串。
 */
inline std::string AssetTypeToString(AssetType type) {
    switch (type) {
        case AssetType::Texture2D: return "Texture2D";
        case AssetType::TextureCubeMap: return "TextureCubeMap";
        case AssetType::Material: return "Material";
        case AssetType::Model: return "Model";
        case AssetType::ModelData: return "ModelData";
        case AssetType::Shader: return "Shader";
        case AssetType::Unknown: return "Unknown";
        case AssetType::COUNT: return "COUNT (Invalid Type)"; // COUNT 不应被直接使用
        default: return "Invalid AssetType"; // 未知或未处理的类型
    }
}

#endif