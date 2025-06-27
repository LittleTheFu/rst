#include "uiSceneData.h"
#include "scene.h"
#include "sceneObject.h"
#include "pointLight.h"

UiSceneData UiSceneData::create(std::shared_ptr<Scene> scene)
{
    UiSceneData data;

    data.sceneData.push_back({"Final Composite", 0});
    data.sceneData.push_back({"", -1});
    data.sceneData.push_back({"Position (G-Buf)", 1});
    data.sceneData.push_back({"Normal (G-Buf)", 2});
    data.sceneData.push_back({"Albedo (G-Buf)", 3});
    data.sceneData.push_back({"Roughness (G-Buf)", 4});
    data.sceneData.push_back({"Metallic (G-Buf)", 5});
    data.sceneData.push_back({"AO (G-Buf)", 6});
    data.sceneData.push_back({"", -1});
    data.sceneData.push_back({"Direct Light", 7});
    data.sceneData.push_back({"IBL Light", 8});
    data.sceneData.push_back({"Combined Deferred", 9});
    data.sceneData.push_back({"SSR Reflection", 10});
    data.sceneData.push_back({"", -1});
    data.sceneData.push_back({"Shadow Current Depth", 11});
    data.sceneData.push_back({"Shadow Closest Depth", 12});
    data.sceneData.push_back({"Brightness Mask", 13});
    data.sceneData.push_back({"God Ray", 14});
    data.sceneData.push_back({"", -1});

    data.onRenderModeChanged = [scene](int mode)
    { 
        scene->setRenderFlag(mode); 
    };

    data.allSceneObjects = scene->getAllSceneObjects(); 

    data.onObjectSelectedFromUI = [scene](ISceneObject* obj)
    { 
        scene->setSelectedObject(obj); 
    };

    data.pointLight = scene->getPointLight(); 

    return data;
}
