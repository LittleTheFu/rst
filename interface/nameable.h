#ifndef INAMABLE_H
#define INAMABLE_H

#include <string> // 包含 std::string 类型

/**
 * @brief INamable 是一个接口类，用于定义任何可被命名的对象。
 * 所有需要拥有一个名称并提供获取该名称方法的类都应继承此接口。
 */
class INamable {
public:
    /**
     * @brief 获取对象的名称。
     * @return 对象的名称（const 引用），确保无法通过此方法修改名称，并避免不必要的拷贝。
     */
    virtual const std::string& getName() const = 0; // 纯虚函数，要求所有派生类必须实现

    /**
     * @brief 虚析构函数。
     * 作为接口，通常不需要有复杂的析构逻辑，但虚析构函数是必需的，
     * 以确保通过基类（INamable*）指针删除派生类对象时，能够正确调用派生类的析构函数，
     * 防止内存泄漏。
     */
    virtual ~INamable() = default; // 使用 = default; 告诉编译器生成默认的虚析构函数
};

#endif // INAMABLE_H