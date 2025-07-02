#pragma once

#include <RmlUi/Core/SystemInterface.h>
#include <SDL.h>     // 用于 SDL_GetTicks() 获取时间
#include <iostream>  // 用于 std::cout 日志

class MySystemInterface : public Rml::SystemInterface
{
public:
    MySystemInterface();
    ~MySystemInterface() = default;

    // Rml::SystemInterface 虚函数实现
    double GetElapsedTime() override;
    int TranslateString(Rml::String& translated, const Rml::String& input) override;
    void JoinPath(Rml::String& translated_path, const Rml::String& document_path, const Rml::String& path) override;
    bool LogMessage(Rml::Log::Type type, const Rml::String& message) override;
    void SetMouseCursor(const Rml::String& cursor_name) override;
    void SetClipboardText(const Rml::String& text) override;
    void GetClipboardText(Rml::String& text) override;
    void ActivateKeyboard(Rml::Vector2f caret_position, float line_height) override;
    void DeactivateKeyboard() override;
};