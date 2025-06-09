// GLException.h
#ifndef GL_EXCEPTION_H
#define GL_EXCEPTION_H

#include <stdexcept> // 继承自 std::runtime_error
#include <string>
#include <glad/glad.h> // 可能需要 GLenum

class GLException : public std::runtime_error {
public:
    // 构造函数：接受消息、文件名和行号
    GLException(const std::string& message, const std::string& file, int line)
        : std::runtime_error(
            message + " (File: " + file + ", Line: " + std::to_string(line) + ")"
          ),
          file_(file),
          line_(line),
          glErrorCode_(0) // 默认无 GL 错误码
    {}

    // 构造函数重载：支持传入 GL 错误码
    GLException(const std::string& message, GLenum glErrorCode, const std::string& file, int line)
        : std::runtime_error(
            message + " (GL Error Code: " + std::to_string(glErrorCode) +
            ", File: " + file + ", Line: " + std::to_string(line) + ")"
          ),
          file_(file),
          line_(line),
          glErrorCode_(glErrorCode)
    {}

    // 提供获取额外信息的接口
    const std::string& getFile() const { return file_; }
    int getLine() const { return line_; }
    GLenum getGLErrorCode() const { return glErrorCode_; }

private:
    std::string file_;
    int line_;
    GLenum glErrorCode_;
};

// 辅助宏：用于简化抛出 GLException 的代码
// 注意：宏通常在调试时非常有用，但要谨慎使用
#define THROW_GL_EXCEPTION(msg) throw GLException(msg, __FILE__, __LINE__)
#define THROW_GL_ERROR_EXCEPTION(msg, glError) throw GLException(msg, glError, __FILE__, __LINE__)

#endif // GL_EXCEPTION_H