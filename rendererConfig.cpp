// RendererConfig.cpp
#include "rendererConfig.h"

// 静态成员的定义和初始化
RendererConfig RendererConfig::instance_;

// 获取单例实例
RendererConfig& RendererConfig::getInstance()
{
    return instance_;
}

// 私有构造函数
RendererConfig::RendererConfig() : windowWidth_(800), windowHeight_(600), windowTitle_("My OpenGL Renderer")
{
    // 可以在这里进行一些初始化操作，如果需要的话
}

// 获取窗口宽度
int RendererConfig::getWindowWidth() const
{
    return windowWidth_;
}

// 设置窗口宽度
void RendererConfig::setWindowWidth(int width)
{
    windowWidth_ = width;
}

// 获取窗口高度
int RendererConfig::getWindowHeight() const
{
    return windowHeight_;
}

// 设置窗口高度
void RendererConfig::setWindowHeight(int height)
{
    windowHeight_ = height;
}

// 获取窗口标题
const std::string& RendererConfig::getWindowTitle() const
{
    return windowTitle_;
}

// 设置窗口标题
void RendererConfig::setWindowTitle(const std::string& title)
{
    windowTitle_ = title;
}