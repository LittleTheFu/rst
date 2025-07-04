#ifndef ECS_COMPONENTS_H
#define ECS_COMPONENTS_H

#include <string> // 用于 RenderComponent 和 NameComponent 中的字符串

// PositionComponent: 表示实体在3D空间中的位置
// 这是最常见的组件之一
struct PositionComponent {
    float x = 0.0f; // 默认值，如果创建时未指定，则为0
    float y = 0.0f;
    float z = 0.0f;
};

// VelocityComponent: 表示实体的速度或移动方向
// 通常与PositionComponent一起使用
struct VelocityComponent {
    float vx = 0.0f; // 沿X轴的速度
    float vy = 0.0f; // 沿Y轴的速度
    float vz = 0.0f; // 沿Z轴的速度
};

// RenderComponent: 包含渲染所需的数据
// 这里简化为只包含模型文件的路径
// 在你的实际项目中，可能会包含 MeshID, MaterialID, TextureID 等
struct RenderComponent {
    std::string modelPath; // 例如 "models/cube.obj"
};

// NameComponent: 一个简单的组件，用于给实体一个可读的名称
struct NameComponent {
    std::string name;
};

// 你可以根据需要添加更多组件，例如：
// struct HealthComponent { int currentHealth; int maxHealth; };
// struct PhysicsBodyComponent { /* JoltPhysics BodyID */ };
// struct PlayerInputComponent {}; // 可以是一个空结构体，只做标记用

#endif // ECS_COMPONENTS_H