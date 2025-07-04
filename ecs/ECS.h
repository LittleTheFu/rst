#ifndef ECS_ECS_H
#define ECS_ECS_H

#include "Types.h"      // 包含 EntityID 和 GetNextEntityID()
#include "Components.h" // 包含所有具体的组件结构
#include "ISystem.h"    // 包含 ISystem 基类

#include <unordered_map> // 用于存储组件，每个组件类型一个map
#include <vector>        // 用于存储系统指针
#include <memory>        // unique_ptr 用于管理系统实例的生命周期
#include <typeindex>     // C++11 用于运行时获取类型信息，这里用于SystemManager
#include <iostream>

// ECS 类的核心职责：
// 1. 管理实体ID的生命周期（创建、销毁）。
// 2. 存储和管理所有组件的实例。
// 3. 管理和运行所有注册的系统。
class ECS {
public:
    // --- 实体管理 ---

    // 创建一个新实体，返回其唯一的ID
    EntityID CreateEntity() {
        EntityID newId = GetNextEntityID(); // 从全局函数获取一个新ID
        // 在更完善的ECS中，这里还会维护一个活跃实体列表，并处理ID回收
        // 简化版暂时忽略这些
        std::cout << "ECS: Created Entity: " << newId << std::endl;
        return newId;
    }

    // 销毁一个实体：从所有组件存储中移除该实体对应的组件
    // 注意：这个简化版需要手动为每种组件类型编写移除逻辑
    void DestroyEntity(EntityID entity) {
        // 尝试从各个组件Map中移除这个实体对应的组件
        // 如果实体没有某个组件，.erase() 不会做任何事，也不会报错
        positions.erase(entity);
        velocities.erase(entity);
        renders.erase(entity);
        names.erase(entity);
        // ... (如果你有更多组件类型，你需要在这里手动添加 .erase(entity); )

        std::cout << "ECS: Destroyed Entity: " << entity << std::endl;
        // 真实ECS会回收这个EntityID以便将来重用
    }

    // --- 组件管理 ---

    // 添加组件：为指定实体添加一个特定类型的组件
    // 注意：这里需要为每种组件类型手动编写一个 AddComponent 重载
    void AddComponent(EntityID entity, const PositionComponent& comp) {
        positions[entity] = comp;
        std::cout << "ECS: Added PositionComponent to Entity " << entity << std::endl;
    }
    void AddComponent(EntityID entity, const VelocityComponent& comp) {
        velocities[entity] = comp;
        std::cout << "ECS: Added VelocityComponent to Entity " << entity << std::endl;
    }
    void AddComponent(EntityID entity, const RenderComponent& comp) {
        renders[entity] = comp;
        std::cout << "ECS: Added RenderComponent to Entity " << entity << std::endl;
    }
    void AddComponent(EntityID entity, const NameComponent& comp) {
        names[entity] = comp;
        std::cout << "ECS: Added NameComponent to Entity " << entity << std::endl;
    }
    // ... (如果你有更多组件类型，你需要在这里手动添加对应的 AddComponent 重载)

    // 获取组件：获取指定实体上的特定类型组件的引用（可修改）
    // 注意：如果实体没有这个组件，.at() 会抛出 std::out_of_range 异常
    PositionComponent& GetPositionComponent(EntityID entity) { return positions.at(entity); }
    VelocityComponent& GetVelocityComponent(EntityID entity) { return velocities.at(entity); }
    RenderComponent& GetRenderComponent(EntityID entity) { return renders.at(entity); }
    NameComponent& GetNameComponent(EntityID entity) { return names.at(entity); }
    // ... (如果你有更多组件类型，你需要在这里手动添加对应的 GetComponent 方法)


    // 检查是否有组件：判断指定实体是否拥有特定类型的组件
    bool HasPositionComponent(EntityID entity) const { return positions.count(entity) > 0; }
    bool HasVelocityComponent(EntityID entity) const { return velocities.count(entity) > 0; }
    bool HasRenderComponent(EntityID entity) const { return renders.count(entity) > 0; }
    bool HasNameComponent(EntityID entity) const { return names.count(entity) > 0; }
    // ... (如果你有更多组件类型，你需要在这里手动添加对应的 HasComponent 方法)


    // 移除组件：从指定实体上移除特定类型的组件
    void RemovePositionComponent(EntityID entity) {
        positions.erase(entity);
        std::cout << "ECS: Removed PositionComponent from Entity " << entity << std::endl;
    }
    void RemoveVelocityComponent(EntityID entity) {
        velocities.erase(entity);
        std::cout << "ECS: Removed VelocityComponent from Entity " << entity << std::endl;
    }
    void RemoveRenderComponent(EntityID entity) {
        renders.erase(entity);
        std::cout << "ECS: Removed RenderComponent from Entity " << entity << std::endl;
    }
    void RemoveNameComponent(EntityID entity) {
        names.erase(entity);
        std::cout << "ECS: Removed NameComponent from Entity " << entity << std::endl;
    }
    // ... (如果你有更多组件类型，你需要在这里手动添加对应的 RemoveComponent 方法)


    // --- 系统管理 ---

    // 添加一个系统到ECS中
    template<typename T> // 使用模板，以便添加任何继承自ISystem的具体系统
    void AddSystem() {
        static_assert(std::is_base_of<ISystem, T>::value, "Error: System must inherit from ISystem.");
        // 使用 unique_ptr 来管理系统对象的生命周期
        systems.push_back(std::make_unique<T>());
        std::cout << "ECS: Added System: " << typeid(T).name() << std::endl;
    }

    // 更新所有注册的系统
    void UpdateSystems(float deltaTime) {
        // 遍历所有系统，并调用它们的 Update 方法
        // 将当前ECS实例的引用传递给每个系统，以便系统可以操作组件
        for (const auto& system : systems) {
            system->Update(*this, deltaTime);
        }
    }

    // --- 提供给系统的组件数据访问方法 ---
    // 为了让系统能够遍历所有组件，我们提供对内部map的只读访问
    // 注意：System::Update里如果需要修改组件，不能用const引用
    // 对于这个简化版，我们将直接提供可修改的引用给系统，在Systems.h中需要相应调整。
    // 在真实ECS中，这里会是更复杂的"视图"或"迭代器"
    const std::unordered_map<EntityID, PositionComponent>& GetAllPositionComponents() const { return positions; }
    const std::unordered_map<EntityID, VelocityComponent>& GetAllVelocityComponents() const { return velocities; }
    const std::unordered_map<EntityID, RenderComponent>& GetAllRenderComponents() const { return renders; }
    const std::unordered_map<EntityID, NameComponent>& GetAllNameComponents() const { return names; }

    // 由于系统的Update方法需要修改组件，我们还需要提供非const的访问器
    // 这是一种非常简化的处理方式，在生产ECS中不会这样直接暴露内部结构
    std::unordered_map<EntityID, PositionComponent>& GetMutablePositionComponents() { return positions; }
    std::unordered_map<EntityID, VelocityComponent>& GetMutableVelocityComponents() { return velocities; }
    std::unordered_map<EntityID, VelocityComponent>& GetMutableRenderComponents() { return renders; } // 修正这里，应该是renders
    std::unordered_map<EntityID, NameComponent>& GetMutableNameComponents() { return names; }


private:
    // --- 组件存储 ---
    // 这是ECS核心的数据存储。
    // 每个 `std::unordered_map` 存储一种特定类型的组件实例。
    // Key是 EntityID，Value是组件的实际数据。
    std::unordered_map<EntityID, PositionComponent> positions;
    std::unordered_map<EntityID, VelocityComponent> velocities;
    std::unordered_map<EntityID, RenderComponent> renders;
    std::unordered_map<EntityID, NameComponent> names;
    // ... (如果你有更多组件类型，你需要在私有成员中手动添加对应的unordered_map)

    // --- 系统存储 ---
    // 存储所有注册的系统。使用 unique_ptr 确保内存安全。
    std::vector<std::unique_ptr<ISystem>> systems;
};

#endif // ECS_ECS_H