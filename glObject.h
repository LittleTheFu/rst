// GLObject.h
#ifndef GL_OBJECT_H
#define GL_OBJECT_H

#include <glad/glad.h> // 确保包含了 GLAD 头文件，以便使用 GLuint

// 前向声明，表示这是一个抽象基类
class GLObject {
public:
    // 获取 OpenGL 资源的 ID。
    // 声明但不在头文件中定义。
    GLuint id() const;

    // --- 禁止拷贝和赋值操作 ---
    // 这些操作符的声明保持不变，因为它们是类接口的一部分。
    GLObject(const GLObject&) = delete;
    GLObject& operator=(const GLObject&) = delete;

protected:
    // 保护的 ID 成员：
    // 仍然在头文件中声明，因为它是类的成员变量。
    GLuint id_ = 0;

    // 保护的默认构造函数：
    // 声明但不在头文件中定义。
    GLObject();

    // 纯虚析构函数：
    // 声明为纯虚函数，但其定义（即使是空的）必须提供。
    // 其定义将放在 GLObject.cpp 中。
    virtual ~GLObject();

    // 纯虚函数：由派生类实现具体的 OpenGL 资源删除逻辑。
    // 声明保持不变。
    virtual void deleteGlResource();
};

#endif // GL_OBJECT_H