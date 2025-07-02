
// RmlUiSystemInterface.cpp
#include "RmlUiSystemInterface.h"
#include <iostream>
#include <SDL_clipboard.h> // 用于 SDL 剪贴板功能
#include <SDL_mouse.h>     // 用于 SDL 鼠标光标功能

RmlUiSystemInterface::RmlUiSystemInterface() {
    performanceFrequency_ = SDL_GetPerformanceFrequency();
    initialTicks_ = SDL_GetPerformanceCounter();

    // 初始化 SDL 鼠标光标
    defaultCursor_ = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    textCursor_ = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
    // 可以根据需要创建更多光标
}

RmlUiSystemInterface::~RmlUiSystemInterface() {
    // 销毁 SDL 鼠标光标
    if (defaultCursor_) {
        SDL_FreeCursor(defaultCursor_);
        defaultCursor_ = nullptr;
    }
    if (textCursor_) {
        SDL_FreeCursor(textCursor_);
        textCursor_ = nullptr;
    }
}

double RmlUiSystemInterface::GetElapsedTime() {
    Uint64 currentTicks = SDL_GetPerformanceCounter();
    return (double)(currentTicks - initialTicks_) / performanceFrequency_;
}

void RmlUiSystemInterface::SetClipboardText(const Rml::String& text) {
    SDL_SetClipboardText(text.c_str());
}

void RmlUiSystemInterface::GetClipboardText(Rml::String& text) {
    char* clipboard_text = SDL_GetClipboardText();
    if (clipboard_text) {
        text = clipboard_text;
        SDL_free(clipboard_text);
    } else {
        text = "";
    }
}

bool RmlUiSystemInterface::LogMessage(Rml::Log::Type type, const Rml::String& message) {
    switch (type) {
        case Rml::Log::Type::LT_ALWAYS:
        case Rml::Log::Type::LT_INFO:
            std::cout << "[RmlUi INFO] " << message << std::endl;
            break;
        case Rml::Log::Type::LT_WARNING:
            std::cout << "[RmlUi WARNING] " << message << std::endl;
            break;
        case Rml::Log::Type::LT_ERROR:
            std::cerr << "[RmlUi ERROR] " << message << std::endl;
            break;
        case Rml::Log::Type::LT_ASSERT:
            std::cerr << "[RmlUi ASSERT] " << message << std::endl;
            break;
        default:
            std::cout << "[RmlUi LOG] " << message << std::endl;
            break;
    }

    //quick and dirty
    return true;
}

void RmlUiSystemInterface::SetMouseCursor(const Rml::String& cursor_name) {
    SDL_Cursor* cursor_to_set = nullptr;

    if (cursor_name == "arrow") {
        cursor_to_set = defaultCursor_;
    } else if (cursor_name == "text") {
        cursor_to_set = textCursor_;
    }
    // TODO: 根据 RmlUi 的光标名称添加更多 SDL_SYSTEM_CURSOR_Xxx
    // 例如：
    // else if (cursor_name == "move") { cursor_to_set = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL); }
    // else if (cursor_name == "pointer") { cursor_to_set = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND); }
    // else if (cursor_name == "resize") { cursor_to_set = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE); }

    if (cursor_to_set) {
        SDL_SetCursor(cursor_to_set);
        // return true;
    }
    // return false;
}

// void RmlUiSystemInterface::SetNativeWindow(void* native_window_handle) {
//     // 在 SDL 中，native_window_handle 通常是 SDL_Window*
//     // RmlUi 内部可能需要这个来做一些平台特定的事情，但对于基本渲染通常不是必需的。
//     // 如果需要，可以在这里存储或使用它。
//     (void)native_window_handle; // 避免未使用参数警告
// }