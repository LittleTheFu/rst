#include "uiSceneData.h"
#include "scene.h"        // 包含 Scene 类定义
#include "sceneObject.h" // 包含 ISceneObject 接口定义
#include "pointLight.h"   // 包含 PointLight 类定义

UiSceneData UiSceneData::create(std::shared_ptr<Scene> scene)
{
    UiSceneData data;

    data.sceneData.push_back({"Final Composite", 0});
    data.sceneData.push_back({"", -1}); // 分隔线
    data.sceneData.push_back({"Position (G-Buf)", 1});
    data.sceneData.push_back({"Normal (G-Buf)", 2});
    data.sceneData.push_back({"Albedo (G-Buf)", 3});
    data.sceneData.push_back({"Roughness (G-Buf)", 4});
    data.sceneData.push_back({"Metallic (G-Buf)", 5});
    data.sceneData.push_back({"AO (G-Buf)", 6});
    data.sceneData.push_back({"", -1}); // 分隔线
    data.sceneData.push_back({"Direct Light", 7});
    data.sceneData.push_back({"IBL Light", 8});
    data.sceneData.push_back({"Combined Deferred", 9});
    data.sceneData.push_back({"SSR Reflection", 10});
    data.sceneData.push_back({"", -1}); // 分隔线
    data.sceneData.push_back({"Shadow Current Depth", 11});
    data.sceneData.push_back({"Shadow Closest Depth", 12});
    data.sceneData.push_back({"Brightness Mask", 13});
    data.sceneData.push_back({"God Ray", 14});
    data.sceneData.push_back({"", -1}); // 分隔线

    // 绑定 onRenderModeChanged 回调
    data.onRenderModeChanged = [scene](int mode)
    { 
        scene->setRenderFlag(mode); 
    };

    // !!! 关键改动 !!!
    // 从 Scene 获取所有 ISceneObject，并填充到 allSceneObjects 中
    data.allSceneObjects = scene->getAllSceneObjects(); 

    // !!! 关键改动 !!!
    // 绑定 onObjectSelectedFromUI 回调
    data.onObjectSelectedFromUI = [scene](ISceneObject* obj)
    { 
        scene->setSelectedObject(obj); // 调用 Scene 的新方法
    };

    // 传递 PointLight 的指针
    data.pointLight = scene->getPointLight(); 

    return data;
}