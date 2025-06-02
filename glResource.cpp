// GLResource.cpp
#include "GLResource.h"
#include <glad/glad.h>


GLResource::GLResource() : id_(0) {}

GLResource::~GLResource() {
    release();
}

GLuint GLResource::id() const {
    return id_;
}

// --- 新增：移动构造函数 ---
GLResource::GLResource(GLResource&& other) noexcept : id_(other.id_) {
    other.id_ = 0; // 转移所有权，清空源对象ID
}

// --- 新增：移动赋值运算符 ---
GLResource& GLResource::operator=(GLResource&& other) noexcept {
    if (this != &other) { // 防止自我赋值
        release(); // 释放当前对象的资源
        id_ = other.id_; // 转移所有权
        other.id_ = 0;   // 清空源对象ID
    }
    return *this;
}

void GLResource::release() {
    // 基类的 release 不做任何操作，由子类根据其资源类型实现
}