// 在一个调试工具头文件 (e.g., debug_utils.h) 中
#pragma once

#include <glad/glad.h>
#include <iostream>
#include <string> // For std::string

// 辅助函数，将 GLenum 错误代码转换为可读字符串 (手动映射，因为 gluErrorString 已废弃)
inline std::string GlErrorToString(GLenum error) {
    switch (error) {
        case GL_NO_ERROR: return "GL_NO_ERROR";
        case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
        case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
        case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
        // 添加其他你可能遇到的错误类型
        default: return "UNKNOWN_ERROR";
    }
}

// 检查 OpenGL 错误的宏/函数
// 仅在 DEBUG 模式下编译
#ifdef _DEBUG
#define GL_CHECK_ERROR() \
    do { \
        GLenum error = glGetError(); \
        if (error != GL_NO_ERROR) { \
            std::cerr << "OpenGL Error " << GlErrorToString(error) << " (Code: " << error << ") at " \
                      << __FILE__ << ":" << __LINE__ << std::endl; \
            __debugbreak(); /* For Visual Studio, to break into debugger */ \
            /* 或者其他平台上的断点函数，例如 assert(false); */ \
        } \
    } while (0)
#else
#define GL_CHECK_ERROR() do {} while (0) // 在发布模式下，宏为空操作
#endif // _DEBUG
