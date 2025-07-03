// EventBus.h
#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include <functional>
#include <map>
#include <vector>
#include <typeindex> // 用于获取类型信息

class EventBus {
public:
    static EventBus& GetInstance(); // 单例访问

    // 发布事件
    template<typename EventType>
    void Publish(const EventType& event) {
        std::type_index typeId = typeid(EventType);
        if (subscribers_.count(typeId)) {
            for (const auto& func : subscribers_[typeId]) {
                // 安全地将 std::function 转换为正确的类型并调用
                func(static_cast<const void*>(&event));
            }
        }
    }

    // 订阅事件
    template<typename EventType, typename ListenerFunc>
    void Subscribe(ListenerFunc listener) {
        std::type_index typeId = typeid(EventType);
        // 将 lambda 包装在一个通用的 std::function 中
        // 需要存储一个可擦除类型的函数对象，这里用 void* 作为参数
        subscribers_[typeId].push_back([listener](const void* eventPtr) {
            listener(*static_cast<const EventType*>(eventPtr));
        });
    }

private:
    EventBus() = default;
    ~EventBus() = default;
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    // 存储订阅者，键是事件类型ID，值是函数列表
    // 使用 std::function<void(const void*)> 允许存储不同事件类型的回调
    std::map<std::type_index, std::vector<std::function<void(const void*)>>> subscribers_;
};

#endif // EVENT_BUS_H