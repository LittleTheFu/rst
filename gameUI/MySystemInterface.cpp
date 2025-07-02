#include "MySystemInterface.h"
#include <RmlUi/Core/StringUtilities.h> // for Rml::String
#include <sstream> // For building log messages

MySystemInterface::MySystemInterface() {
    // 构造函数，无需特殊初始化
}

double MySystemInterface::GetElapsedTime()
{
    // SDL_GetTicks() 返回毫秒，转换为秒
    return static_cast<double>(SDL_GetTicks()) / 1000.0;
}

int MySystemInterface::TranslateString(Rml::String& translated, const Rml::String& input)
{
    // 默认不进行翻译。如果您需要国际化/本地化，将来可以在这里实现。
    translated = input;
    return 0; // 表示没有进行翻译
}

void MySystemInterface::JoinPath(Rml::String& translated_path, const Rml::String& document_path, const Rml::String& path)
{
    // 调用 RmlUi 基类的默认 JoinPath 实现。
    // RmlUi 的默认 JoinPath 逻辑通常能正确处理相对路径。
    // MyFileInterface 会在 Open 函数中处理 'resource/' 前缀。
    Rml::SystemInterface::JoinPath(translated_path, document_path, path);
}

bool MySystemInterface::LogMessage(Rml::Log::Type type, const Rml::String& message)
{
    // 根据 RmlUi 的日志类型输出到 std::cout
    std::string type_str = "[UNKNOWN]";
    switch (type) {
        case Rml::Log::LT_ALWAYS:     type_str = "[ALWAYS]"; break;
        case Rml::Log::LT_ERROR:      type_str = "[ERROR]"; break;
        case Rml::Log::LT_WARNING:    type_str = "[WARNING]"; break;
        case Rml::Log::LT_INFO:       type_str = "[INFO]"; break;
        case Rml::Log::LT_DEBUG:      type_str = "[DEBUG]"; break;
        case Rml::Log::LT_ASSERT:     type_str = "[ASSERT]"; break;
    }
    std::cout << "RmlUi " << type_str << ": " << message.c_str() << std::endl;
    return true; // 返回 true 表示消息已处理
}

void MySystemInterface::SetMouseCursor(const Rml::String& /*cursor_name*/)
{
    // 暂时不处理鼠标光标。
}

void MySystemInterface::SetClipboardText(const Rml::String& /*text*/)
{
    // 暂时不处理剪贴板。
}

void MySystemInterface::GetClipboardText(Rml::String& /*text*/)
{
    // 暂时不处理剪贴板。
}

void MySystemInterface::ActivateKeyboard(Rml::Vector2f /*caret_position*/, float /*line_height*/)
{
    // 暂时不处理软键盘。
}

void MySystemInterface::DeactivateKeyboard()
{
    // 暂时不处理软键盘。
}