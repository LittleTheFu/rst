#include "uiSceneData.h"

UiSceneData UiSceneData::create(Scene &scene)
{
    UiSceneData data;

    data.sceneData.push_back({"reset", 0});
    data.sceneData.push_back({"pos", 1});
    data.sceneData.push_back({"norm", 2});
    data.sceneData.push_back({"alb", 3});
    data.sceneData.push_back({"rough", 4});
    data.sceneData.push_back({"metal", 5});
    data.sceneData.push_back({"ao", 6});

    data.onRenderModeChanged = [&scene](int mode)
    { scene.setRenderFlag(mode); };

    return data;
}