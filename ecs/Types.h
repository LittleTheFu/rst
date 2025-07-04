#ifndef ECS_TYPES_H
#define ECS_TYPES_H

// 实体ID，我们使用无符号整型来表示一个唯一的实体
using EntityID = unsigned int;

// 一个简单的函数，用于生成下一个可用的实体ID
// static关键字确保这个变量只在这个编译单元内部可见，并且只初始化一次
// inline关键字允许这个函数在多个编译单元中定义而不会导致链接错误 (C++17)
inline EntityID GetNextEntityID() {
    static EntityID nextId = 0; // 静态变量，只初始化一次，每次调用递增
    return nextId++;
}

#endif // ECS_TYPES_H