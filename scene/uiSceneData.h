#ifndef _UI_SCENE_DATA_H_
#define _UI_SCENE_DATA_H_

#include <string>
#include <vector>
#include <functional>
#include <tuple>
#include "scene.h"

class UiSceneData
{
public:
    std::vector<std::tuple<std::string, int>> sceneData;
    std::function<void(int)> onRenderModeChanged;

    std::vector<Mesh*> allMeshes;
    std::function<void(Mesh*)> onMeshSelectedFromUI;

    PointLight* pointLight;

    static UiSceneData create(Scene& scene);
};

#endif // _UI_SCENE_DATA_H_