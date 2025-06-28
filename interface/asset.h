#ifndef _IASSET_H_
#define _IASSET_H_


#include "assetType.h"
#include <string>   // 用于 std::string
#include <memory>   // 用于 std::shared_ptr，虽然这里不强制，但通常一起使用


// --- IAsset 接口定义 ---
/**
 * @brief IAsset 接口代表了所有可被资源管理器统一管理的抽象资产。
 *
 * 实现此接口的类将拥有一个唯一的ID（通常是文件路径），
 * 并能报告其自身的类型，以便资源管理器能够泛型地处理不同种类的资源。
 */
class IAsset {
public:
    /**
     * @brief 虚析构函数，确保所有继承 IAsset 的派生类能够正确析构。
     * 这是 C++ 中实现多态基类的标准做法。
     */
    virtual ~IAsset() = default;

    /**
     * @brief 获取此资产的唯一标识符。
     *
     * 通常是加载此资产的文件路径（例如 "textures/albedo.png"），
     * 对于内部生成的资产也可以是一个唯一的字符串ID。
     *
     * @return 资产的唯一ID字符串的常量引用。
     */
    virtual const std::string& getID() const = 0;

    /**
     * @brief 获取此资产的类型。
     *
     * 返回一个 AssetType 枚举值，表示资产的具体类型，
     * 这对于类型安全的判断和泛型处理非常有用。
     *
     * @return 资产的 AssetType 枚举值。
     */
    virtual AssetType getType() const = 0;

    /**
     * @brief 获取此资产类型名称的字符串表示。
     *
     * 这是 getType() 的辅助方法，方便调试、日志记录和 UI 显示。
     * 默认实现调用 AssetTypeToString 函数。
     *
     * @return 资产类型名称的 C 风格字符串。
     */
    virtual std::string getTypeString() const {
        return AssetTypeToString(getType());
    }

    // TODO: 未来你可能需要在这里添加其他通用的资产生命周期或状态方法，例如：
    // virtual bool isValid() const { return true; } // 检查资源是否已成功加载并有效
    // virtual void releaseGPUResources() = 0; // 如果你想提供手动释放 GPU 资源的方法
};

#endif