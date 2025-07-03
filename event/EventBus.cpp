#include "EventBus.h" // 包含 EventBus 类的头文件
// 不需要在这里再次包含 <functional>, <map>, <vector>, <typeindex>，因为 EventBus.h 已经包含了它们。

// EventBus 单例的 GetInstance 方法的定义
// 这是链接器在查找 "public: static class EventBus & __cdecl EventBus::GetInstance(void)" 时需要找到的实际函数实现。
EventBus& EventBus::GetInstance() {
    // 使用静态局部变量实现单例模式：
    // - `static` 确保 `instance` 只会被创建一次。
    // - `instance` 在程序首次调用 `GetInstance()` 时初始化。
    // - `instance` 在程序结束时自动销毁（RAII）。
    static EventBus instance; 
    return instance;
}

// 注意：
// EventBus::Publish 和 EventBus::Subscribe 方法是模板函数。
// 根据 C++ 模板的特性，它们的定义（实现）通常需要直接放在头文件 (.h) 中，
// 以便编译器在编译使用这些模板函数的地方时，能够完全看到它们的实现并进行实例化。
// 所以，你不需要在这里重新实现 Publish 和 Subscribe。它们已经正确地放在了 EventBus.h 中。