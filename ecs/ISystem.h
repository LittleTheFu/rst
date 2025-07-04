#ifndef ECS_ISYSTEM_H
#define ECS_ISYSTEM_H

// 因为系统需要操作ECS实例来获取组件，所以这里需要前置声明ECS类
// 避免在ISystem.h中包含ECS.h，造成循环依赖（ECS.h会包含ISystem.h）
class ECS;

// 所有ECS系统的抽象基类
// 所有的具体系统（如 MovementSystem, RenderSystem）都将继承自它
class ISystem {
public:
    // 虚析构函数：确保派生类对象在通过基类指针删除时能够正确析构
    virtual ~ISystem() = default;

    // 纯虚函数：所有派生类系统必须实现这个方法来执行它们的逻辑
    // 它接收一个对ECS实例的引用，以及时间步长deltaTime
    // 通过ECS实例，系统将能够查询实体并访问它们的组件
    virtual void Update(ECS& ecs, float deltaTime) = 0;
};

#endif // ECS_ISYSTEM_H