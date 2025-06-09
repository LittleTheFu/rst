#include "IndexBuffer.h" // 注意大小写与文件名一致
#include <iostream> // For error messages if needed

// 默认构造函数
IndexBuffer::IndexBuffer()
    : GLResource(), count_(0), type_(0) { // 初始化 count_ 和 type_
    glCreateBuffers(1, &id_); // DSA: 直接创建缓冲区 ID
    // 检查错误
    if (id_ == 0) {
        std::cerr << "Error: Failed to create IndexBuffer." << std::endl;
    }
}

// 带数据的构造函数
// 注意：这里需要知道索引的类型，以便正确计算字节大小和用于glDrawElements
// 假设这里只处理 unsigned int
IndexBuffer::IndexBuffer(const void* data, size_t count, GLenum usage)
    : IndexBuffer() // 调用默认构造函数来创建ID
{
    // 默认使用 GL_UNSIGNED_INT 类型，如果需要支持其他类型，需要修改接口
    if (data && count > 0) {
        setDataInternal(data, count, usage, GL_UNSIGNED_INT);
    }
}

// 实现移动构造函数
IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
    : GLResource(std::move(other)), // 移动基类部分（如果 GLResource 支持移动）
      count_(other.count_),
      type_(other.type_)
{
    // 如果 GLResource 没有移动语义，手动转移 id_
    // id_ = other.id_; 
    other.id_ = 0;   // 清空源对象的 ID
    other.count_ = 0; // 清空源对象的 count
    other.type_ = 0;  // 清空源对象的 type
}

// 实现移动赋值运算符
IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept {
    if (this != &other) { // 防止自我赋值
        release(); // 释放当前对象的资源

        id_ = other.id_; // 转移 ID
        count_ = other.count_; // 转移 count
        type_ = other.type_;   // 转移 type

        other.id_ = 0;   // 清空源对象的 ID
        other.count_ = 0; // 清空源对象的 count
        other.type_ = 0;  // 清空源对象的 type
    }
    return *this;
}

// 内部帮助函数，用于设置数据 (DSA 方式)
void IndexBuffer::setDataInternal(const void* data, size_t count, GLenum usage, GLenum type) {
    if (id_ == 0) {
        std::cerr << "Error: Attempted to set data on an uninitialized IndexBuffer." << std::endl;
        return;
    }

    size_t byteSize = 0;
    if (type == GL_UNSIGNED_INT) {
        byteSize = count * sizeof(unsigned int);
    } else if (type == GL_UNSIGNED_SHORT) {
        byteSize = count * sizeof(unsigned short);
    } else if (type == GL_UNSIGNED_BYTE) {
        byteSize = count * sizeof(unsigned char);
    } else {
        std::cerr << "Error: Unsupported index type for IndexBuffer: " << type << std::endl;
        return;
    }

    glNamedBufferData(id_, byteSize, data, usage); // DSA: 直接操作 ID
    
    count_ = count;
    type_ = type;
}


// 移除 bind/unbind 方法，因为我们使用的是 DSA
// 如果你仍需要绑定/解绑，请考虑它们是否应该属于 VertexArray 的职责
// 或者是 IndexBuffer 的一个非虚方法（如果需要绑定 EBO 目标来绘制，但通常是VAO做的）

size_t IndexBuffer::getCount() const {
    return count_;
}

GLenum IndexBuffer::getType() const {
    return type_; 
}

// 实现 GLResource 的 release 纯虚函数
void IndexBuffer::release() {
    if (id_ != 0) {
        glDeleteBuffers(1, &id_);
        id_ = 0;
        count_ = 0; // 清理状态
        type_ = 0;  // 清理状态
    }
}