// ShaderManager.h
#pragma once

#include "shader.h" // 引入 Shader 类
#include "asset.h"    // 引入 IAsset 接口
#include "assetType.h" // 引入 AssetType 枚举

#include <string>           // 用于 std::string
#include <map>              // 用于 std::map 作为缓存容器
#include <memory>           // 用于 std::shared_ptr
#include <iostream>         // 用于日志输出
#include <vector>           // 用于可选的宏定义列表

/**
 * @brief ShaderManager 是一个单例类，负责着色器程序的加载、编译、链接和管理。
 *
 * 它确保每个唯一的着色器程序（由顶点和片段着色器路径确定）在内存中只被加载一次，
 * 并能被应用程序中的多个渲染通道或部分共享。
 */
class ShaderManager {
public:
    /**
     * @brief 获取 ShaderManager 的唯一实例。
     * @return ShaderManager 实例的引用。
     */
    static ShaderManager& getInstance();

    /**
     * @brief 加载或获取一个着色器程序。
     *
     * 如果着色器程序已在缓存中，则直接返回其共享指针；
     * 否则，从文件加载、编译、链接并缓存。
     *
     * @param vertexPath 顶点着色器文件的完整路径。
     * @param fragmentPath 片段着色器文件的完整路径。
     * @return 着色器程序的 std::shared_ptr，如果加载失败则返回 nullptr。
     */
    std::shared_ptr<Shader> loadShader(const std::string& vertexPath, const std::string& fragmentPath);

    /**
     * @brief 加载或获取一个带宏定义的着色器程序。
     *
     * 如果着色器程序已在缓存中，则直接返回其共享指针；
     * 否则，从文件加载、编译、链接并缓存。宏定义会影响着色器编译，因此相同路径不同宏会产生不同着色器。
     *
     * @param vertexPath 顶点着色器文件的完整路径。
     * @param fragmentPath 片段着色器文件的完整路径。
     * @param defines 一个包含宏定义字符串的向量，例如 {"#define USE_LIGHTING", "#define ENABLE_SHADOWS"}
     * @return 着色器程序的 std::shared_ptr，如果加载失败则返回 nullptr。
     */
    std::shared_ptr<Shader> loadShader(const std::string& vertexPath, const std::string& fragmentPath, const std::vector<std::string>& defines);

    /**
     * @brief 尝试从缓存中获取一个已加载的着色器程序。
     * @param key 着色器程序的唯一 ID (通常是文件路径组合)。
     * @return 着色器程序的 std::shared_ptr，如果未找到则返回 nullptr。
     */
    std::shared_ptr<Shader> getShader(const std::string& key);

    /**
     * @brief 卸载指定 Key 的着色器程序。
     *
     * 这将从缓存中移除着色器。如果它是最后一个引用，
     * 着色器对象本身（及其 OpenGL 资源）将被释放。
     *
     * @param key 要卸载着色器程序的 ID。
     */
    void unloadShader(const std::string& key);

    /**
     * @brief 清空所有已缓存的着色器程序。
     *
     * 强制释放所有缓存的着色器。ShaderManager 自身不持有资源的唯一所有权，
     * 但清空 map 会减少 shared_ptr 的引用计数。如果外部没有其他引用，资源将被释放。
     * 这应谨慎使用，通常在程序结束或大规模场景切换时。
     */
    void clearAllShaders();

private:
    // --- 单例模式的私有成员 ---
    ShaderManager();                                     // 私有构造函数
    ~ShaderManager();                                    // 私有析构函数
    ShaderManager(const ShaderManager&) = delete;            // 禁用拷贝构造函数
    ShaderManager& operator=(const ShaderManager&) = delete; // 禁用赋值运算符

    // 内部缓存，Key 是由路径和宏定义组合而成的唯一字符串
    std::map<std::string, std::shared_ptr<Shader>> m_loadedShaders;

    /**
     * @brief 根据顶点和片段路径以及可选的宏定义生成一个唯一的 Key。
     * @param vertexPath 顶点着色器路径。
     * @param fragmentPath 片段着色器路径。
     * @param defines 可选的宏定义列表。
     * @return 唯一标识此着色器程序的字符串 Key。
     */
    std::string generateShaderKey(const std::string& vertexPath, const std::string& fragmentPath, const std::vector<std::string>& defines = {}) const;
};
