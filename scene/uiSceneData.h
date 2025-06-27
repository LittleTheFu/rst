#ifndef UI_SCENE_DATA_H
#define UI_SCENE_DATA_H

#include <vector>
#include <string>
#include <tuple>
#include <functional>
#include <memory> // For std::shared_ptr

#include "sceneObject.h" // 引入 ISceneObject 接口
#include "scene.h"        // 引入 Scene 类，以便访问 PointLight 和获取对象列表
#include "pointLight.h"   // 确保 PointLight 定义被包含，因为 pointLight 是 PointLight* 类型

class UiSceneData {
public:
    // 用于渲染模式选择的元组 (名称, 值)
    std::vector<std::tuple<std::string, int>> sceneData;
    
    // 回调函数，当渲染模式改变时由 UI 调用
    std::function<void(int)> onRenderModeChanged;

    // !!! 关键改动 !!!
    // 存储所有场景对象，现在是 ISceneObject* 类型
    std::vector<ISceneObject*> allSceneObjects; 

    // !!! 关键改动 !!!
    // 当用户在 UI 中选择一个对象时，通知 Scene
    std::function<void(ISceneObject*)> onObjectSelectedFromUI;

    // 指向场景中的 PointLight 实例
    PointLight* pointLight = nullptr; 

public:
    // 静态工厂方法，用于从 Scene 实例创建 UiSceneData
    static UiSceneData create(std::shared_ptr<Scene> scene);
};

#endif // UI_SCENE_DATA_H