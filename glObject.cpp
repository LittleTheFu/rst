// GLObject.cpp
#include "GLObject.h" // 包含对应的头文件

// GLObject 类的默认构造函数定义
// 注意：即使它是 default 构造函数，当在头文件中只有声明时，
// 也需要在源文件中给出定义 (或再次使用 = default)。
GLObject::GLObject() = default;

// GLObject 类的析构函数定义
// 纯虚析构函数也必须有一个定义（即使是空的），因为在多态删除时它会被调用。
GLObject::~GLObject() {
    // 这里的 id_ 检查和 deleteGlResource() 调用保持不变，
    // 因为这是基类析构函数的核心逻辑。
    if (id_ != 0) {
        deleteGlResource();
    }
}

// GLObject 类的 id() 方法定义
// 这是一个简单的 getter，现在将其定义放到 .cpp 文件中。
GLuint GLObject::id() const {
    return id_;
}

void GLObject::deleteGlResource() {
    // 这个函数体通常是空的，或者可以包含一个调试断言/日志，
    // 以防万一在不应该被调用的情况下被调用（这通常是设计错误）。
    // 例如：assert(false && "Pure virtual deleteGlResource called from base class!");
}