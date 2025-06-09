#ifndef _GL_DEBUG_H_
#define _GL_DEBUG_H_

// 在你的 GLException.h 文件中，或者专门的 OpenGL 调试工具头文件中
#ifdef _DEBUG // 通常只在调试模式下启用此功能
#include <iostream>
#include "SDL_opengl.h"

// 辅助函数，将 GLenum 错误码转换为可读的字符串 (可选，但非常有用)
// 你需要自己实现这个函数，或者使用 GLAD/GLEW 提供的工具
inline const char *GLErrorToString(GLenum error)
{
    switch (error)
    {
    case GL_NO_ERROR:
        return "GL_NO_ERROR";
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_STACK_OVERFLOW:
        return "GL_STACK_OVERFLOW";
    case GL_STACK_UNDERFLOW:
        return "GL_STACK_UNDERFLOW";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    // 添加更多你可能遇到的错误码
    default:
        return "UNKNOWN_GL_ERROR";
    }
}

// #define GL_CHECK_ERROR()                                                                \
//     do                                                                                  \
//     {                                                                                   \
//         GLenum err;                                                                     \
//         while ((err = glGetError()) != GL_NO_ERROR)                                     \
//         {                                                                               \
//             std::cerr << "OpenGL Error (GL_CHECK_ERROR): "                              \
//                       << GLErrorToString(err) << " (" << err << ")"                     \
//                       << " at " << __FILE__ << ":" << __LINE__ << std::endl;            \
//             /* 如果你希望在发现任何 GL 错误时立即抛出异常，可以在这里添加 */            \
//             /* throw GLException("Uncaught OpenGL error.", err, __FILE__, __LINE__); */ \
//         }                                                                               \
//     } while (0)
// #else
// #define GL_CHECK_ERROR() \
//     do                   \
//     {                    \
//     } while (0) // 在发布模式下禁用
// #endif
#endif

#endif // _GL_DEBUG_H_