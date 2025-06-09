#include "VertexBuffer.h" // 注意大小写与文件名一致

// 默认构造函数（可选）
VertexBuffer::VertexBuffer() : GLResource() {
    glCreateBuffers(1, &id_); // DSA: 直接创建缓冲区 ID
}

// 带数据的构造函数
VertexBuffer::VertexBuffer(const void* data, GLsizeiptr size, GLenum usage) : GLResource() {
    glCreateBuffers(1, &id_); // DSA: 直接创建缓冲区 ID
    setData(data, size, usage); // 调用 setData 来填充数据
}

// 实现移动构造函数
VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
    : GLResource() // 先构造基类部分
{
    id_ = other.id_; // 转移 ID
    other.id_ = 0;   // 清空源对象的 ID，防止其析构时释放
}

// 实现移动赋值运算符
VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept {
    if (this != &other) { // 防止自我赋值
        release(); // 释放当前对象的资源

        id_ = other.id_; // 转移 ID
        other.id_ = 0;   // 清空源对象的 ID

        // 不需要调用基类的赋值运算符，因为我们直接操作了 id_
    }
    return *this;
}

// 使用 DSA 方式上传数据，无需绑定
void VertexBuffer::setData(const void* data, GLsizeiptr size, GLenum usage) {
    // glNamedBufferData 会重新分配存储
    glNamedBufferData(id_, size, data, usage);
}

// 使用 DSA 方式更新部分数据，无需绑定
void VertexBuffer::updateSubData(const void* data, GLintptr offset, GLsizeiptr size) {
    glNamedBufferSubData(id_, offset, size, data);
}

// 使用 DSA 方式映射缓冲区
void* VertexBuffer::mapBuffer(GLenum access) {
    return glMapNamedBuffer(id_, access);
}

// 使用 DSA 方式解映射缓冲区
void VertexBuffer::unmapBuffer() {
    glUnmapNamedBuffer(id_);
}

// 实现 GLResource 的 release 纯虚函数
void VertexBuffer::release() {
    if (id_ != 0) {
        glDeleteBuffers(1, &id_);
        id_ = 0;
    }
}