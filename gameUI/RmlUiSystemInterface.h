// RmlUiSystemInterface.h
#ifndef RMLUI_SYSTEM_INTERFACE_H
#define RMLUI_SYSTEM_INTERFACE_H

#include <RmlUi/Core/SystemInterface.h>
#include <SDL.h> // 用于剪贴板和鼠标光标

/**
 * @brief RmlUiSystemInterface 实现了 RmlUi 的 SystemInterface，
 * 处理系统级功能，如时间、剪贴板和鼠标光标。
 */
class RmlUiSystemInterface : public Rml::SystemInterface {
public:
    RmlUiSystemInterface();
    virtual ~RmlUiSystemInterface();

    /**
     * @brief 获取自上次调用以来经过的时间（以秒为单位）。
     * @return 经过的时间（秒）。
     */
    double GetElapsedTime() override;

    /**
     * @brief 设置剪贴板文本。
     * @param text 要设置到剪贴板的文本。
     */
    void SetClipboardText(const Rml::String& text) override;

    /**
     * @brief 获取剪贴板文本。
     * @return 剪贴板中的文本。
     */
    void GetClipboardText(Rml::String& text) override;

    /**
     * @brief 打印日志消息。
     * @param type 消息类型（信息、警告、错误）。
     * @param message 消息内容。
     */

    bool LogMessage(Rml::Log::Type type, const Rml::String& message) override;


    void SetMouseCursor(const Rml::String& cursor_name) override;

    /**
     * @brief 设置原生窗口句柄。
     * @param native_window_handle 窗口句柄。
     */
    // void SetNativeWindow(void* native_window_handle) override;

private:
    Uint64 performanceFrequency_; // SDL 性能计数器频率
    Uint64 initialTicks_;         // 初始时间戳

    SDL_Cursor* defaultCursor_; // 默认光标
    SDL_Cursor* textCursor_;    // 文本输入光标
    // 可以根据需要添加更多光标类型
};

#endif // RMLUI_SYSTEM_INTERFACE_H


