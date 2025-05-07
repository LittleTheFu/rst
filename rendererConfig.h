// RendererConfig.h
#ifndef RENDERER_CONFIG_H
#define RENDERER_CONFIG_H

#include <string>

class RendererConfig
{
public:
    // 获取单例实例
    static RendererConfig& getInstance();

    // 获取窗口宽度
    int getWindowWidth() const;

    // 设置窗口宽度
    void setWindowWidth(int width);

    // 获取窗口高度
    int getWindowHeight() const;

    // 设置窗口高度
    void setWindowHeight(int height);

    // 获取窗口标题
    const std::string& getWindowTitle() const;

    // 设置窗口标题
    void setWindowTitle(const std::string& title);

private:
    // 私有构造函数，防止外部直接创建实例
    RendererConfig();

    // 禁用拷贝构造函数和拷贝赋值运算符
    RendererConfig(const RendererConfig&) = delete;
    RendererConfig& operator=(const RendererConfig&) = delete;

    int windowWidth_;
    int windowHeight_;
    std::string windowTitle_;

    // 静态实例
    static RendererConfig instance_;
};

#endif // RENDERER_CONFIG_H