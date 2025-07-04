#ifndef ECS_SYSTEMS_H
#define ECS_SYSTEMS_H

#include "ISystem.h" // 包含系统基类
#include "ECS.h"     // 需要包含ECS类来访问其组件和实体方法
#include "Components.h" // 需要包含组件定义，以便系统知道如何操作它们

#include <iostream>
#include <string> // 用于 NameComponent 的处理

// MovementSystem: 负责处理所有具备移动能力的实体
// 它会查找拥有 PositionComponent 和 VelocityComponent 的实体，并更新它们的位置
class MovementSystem : public ISystem {
public:
    void Update(ECS& ecs, float deltaTime) override {
        std::cout << "\n--- Running MovementSystem ---" << std::endl;

        // 获取所有PositionComponent的集合（这里是unordered_map）
        // 注意：由于ECS::GetMutablePositionComponents()返回的是非const引用，我们可以直接修改map中的组件
        // 但我们不能直接迭代并修改原始的map，因为在迭代时删除元素可能导致迭代器失效
        // 所以我们通常会先收集需要修改的实体ID，或者更安全地遍历
        // 在最简化版本中，我们直接遍历GetMutablePositionComponents()返回的map，
        // 并在内部通过ecs.HasVelocityComponent(entity)检查是否有另一个组件。
        // 这种方式效率很低，但最易于理解。

        // 临时拷贝一份PositionComponent的实体ID，以安全地迭代
        // 避免在循环中修改底层map导致迭代器失效问题
        std::vector<EntityID> entitiesWithPosition;
        for (auto const& pair : ecs.GetMutablePositionComponents()) {
            entitiesWithPosition.push_back(pair.first);
        }

        for (EntityID entity : entitiesWithPosition) {
            // 检查实体是否同时拥有 PositionComponent 和 VelocityComponent
            if (ecs.HasPositionComponent(entity) && ecs.HasVelocityComponent(entity)) {
                // 获取 PositionComponent 和 VelocityComponent 的引用
                PositionComponent& position = ecs.GetMutablePositionComponents().at(entity);
                VelocityComponent& velocity = ecs.GetMutableVelocityComponents().at(entity);

                // 更新位置
                position.x += velocity.vx * deltaTime;
                position.y += velocity.vy * deltaTime;
                position.z += velocity.vz * deltaTime;

                // 尝试获取名称，以便打印更友好的信息
                std::string entityName = "Unnamed Entity";
                if (ecs.HasNameComponent(entity)) {
                    entityName = ecs.GetNameComponent(entity).name;
                }

                std::cout << "  Entity '" << entityName << "' (" << entity << ") moved to ("
                          << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
            }
        }
    }
};

// RenderSystem: 负责处理所有需要渲染的实体
// 它会查找拥有 PositionComponent 和 RenderComponent 的实体，并打印其渲染信息
class RenderSystem : public ISystem {
public:
    void Update(ECS& ecs, float deltaTime) override {
        std::cout << "\n--- Running RenderSystem ---" << std::endl;

        // 临时拷贝一份RenderComponent的实体ID，以安全地迭代
        std::vector<EntityID> entitiesWithRender;
        for (auto const& pair : ecs.GetMutableRenderComponents()) { // 注意：这里是GetMutableRenderComponents()
            entitiesWithRender.push_back(pair.first);
        }

        for (EntityID entity : entitiesWithRender) {
            // 检查实体是否同时拥有 PositionComponent 和 RenderComponent
            if (ecs.HasPositionComponent(entity) && ecs.HasRenderComponent(entity)) {
                // 获取 PositionComponent 和 RenderComponent 的引用
                PositionComponent& position = ecs.GetPositionComponent(entity); // 渲染通常不修改位置，所以用constGet也可以
                RenderComponent& render = ecs.GetRenderComponent(entity);

                // 尝试获取名称
                std::string entityName = "Unnamed Entity";
                if (ecs.HasNameComponent(entity)) {
                    entityName = ecs.GetNameComponent(entity).name;
                }

                std::cout << "  Rendering '" << entityName << "' (Entity " << entity << ") at ("
                          << position.x << ", " << position.y << ", " << position.z << ") with model: "
                          << render.modelPath << std::endl;
            }
        }
    }
};

// 你可以添加更多系统，例如：
// class PlayerInputSystem : public ISystem { /* ... */ };
// class PhysicsSystem : public ISystem { /* ... */ };
// class AnimationSystem : public ISystem { /* ... */ };

#endif // ECS_SYSTEMS_H