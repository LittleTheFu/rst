#ifndef ISCENEOBJECT_H
#define ISCENEOBJECT_H

// 包含所有组合接口的头文件
#include "renderable.h"   // 包含 IRenderable 接口的定义
#include "transformable.h"// 包含 ITransformable 接口的定义
#include "nameable.h"      // 包含 INamable 接口的定义

// 假设你可能还需要一个方法来获取对象的边界体积，如果需要的话可以添加
// #include "BoundingVolume.h" // 如果你定义了 BoundingVolume 或 AABB

/**
 * @brief ISceneObject 是一个组合接口，定义了场景中所有可交互、可渲染、
 * 可变换且可命名的对象的通用行为。
 *
 * 任何需要在场景中被渲染、被操作（如移动、缩放、旋转）并具有名称的类，
 * 都应该继承此接口。
 */
class ISceneObject : public IRenderable, public ITransformable, public INamable {
public:
    // ISceneObject 作为组合接口，通常不需要定义自己的纯虚函数，
    // 因为它已经通过继承，要求派生类实现其所有父接口的纯虚函数。

    // 如果你将来需要场景中所有对象都具备某个特定功能，但这个功能不属于
    // IRenderable, ITransformable, INamable 中的任何一个，你可以在这里添加纯虚函数。
    // 例如：
    // virtual BoundingVolume* getBoundingVolume() const = 0; // 获取对象的边界体积（世界空间）

    /**
     * @brief 虚析构函数。
     * 作为多态基类，虚析构函数是必需的，以确保通过 ISceneObject* 指针
     * 删除派生类对象时，能够正确调用派生类的析构函数，防止内存泄漏。
     */
    virtual ~ISceneObject() = default; // 使用 = default; 告诉编译器生成默认的虚析构函数
};

#endif // ISCENEOBJECT_H