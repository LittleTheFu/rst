#ifndef INDEX_BUFFER_H
#define INDEX_BUFFER_H

#include "GLResource.h"
#include <glad/glad.h>
#include <cstddef> // For size_t
#include <utility> // For std::move

class IndexBuffer : public GLResource {
public:
    // 默认构造函数（可选）：如果需要先创建再填充数据
    IndexBuffer(); 
    
    // 构造函数：直接使用 DSA 函数创建和初始化缓冲区
    // data: 指向索引数据的指针
    // count: 索引的数量 (不是字节大小)
    // usage: 缓冲区的预期使用模式 (GL_STATIC_DRAW, GL_DYNAMIC_DRAW等)
    IndexBuffer(const void* data, size_t count, GLenum usage);

    // 获取索引的数量，用于 glDrawElements
    size_t getCount() const;
    // 获取索引的类型（例如 GL_UNSIGNED_INT, GL_UNSIGNED_SHORT）
    GLenum getType() const;

    // 禁用复制构造函数和赋值运算符，防止资源共享导致的 double free
    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;

    // 移动构造函数和移动赋值运算符（推荐实现）
    IndexBuffer(IndexBuffer&& other) noexcept;
    IndexBuffer& operator=(IndexBuffer&& other) noexcept;

protected:
    // 实现 GLResource 的 release 纯虚函数
    void release() override;

private:
    size_t count_; // 存储索引的数量
    GLenum type_;  // 存储索引的类型（GL_UNSIGNED_INT, GL_UNSIGNED_SHORT 等）

    // 内部帮助函数，用于设置数据
    void setDataInternal(const void* data, size_t count, GLenum usage, GLenum type);
};

#endif // INDEX_BUFFER_H