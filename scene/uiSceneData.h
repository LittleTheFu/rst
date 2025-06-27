#ifndef UI_SCENE_DATA_H
#define UI_SCENE_DATA_H

#include <vector>
#include <string>
#include <tuple>
#include <functional>
#include <memory>

#include "sceneObject.h"
#include "scene.h"
#include "pointLight.h"

class UiSceneData {
public:
    std::vector<std::tuple<std::string, int>> sceneData;
    std::function<void(int)> onRenderModeChanged;

    std::vector<ISceneObject*> allSceneObjects; 
    std::function<void(ISceneObject*)> onObjectSelectedFromUI;

    PointLight* pointLight = nullptr; 

public:
    static UiSceneData create(std::shared_ptr<Scene> scene);
};

#endif