#include "uiSceneData.h"

UiSceneData UiSceneData::create(Scene &scene)
{
    UiSceneData data;

    data.sceneData.push_back({"reset", 0});
    data.sceneData.push_back({"", -1});// 分隔线
    data.sceneData.push_back({"pos", 1});
    data.sceneData.push_back({"norm", 2});
    data.sceneData.push_back({"alb", 3});
    data.sceneData.push_back({"rough", 4});
    data.sceneData.push_back({"metal", 5});
    data.sceneData.push_back({"ao", 6});
    data.sceneData.push_back({"", -1});// 分隔线
    data.sceneData.push_back({"light", 7});
    data.sceneData.push_back({"ibl", 8});
    data.sceneData.push_back({"combined", 9});
    data.sceneData.push_back({"ssr", 10});
    data.sceneData.push_back({"", -1});// 分隔线
    data.sceneData.push_back({"shd_cur_dep", 11});
    data.sceneData.push_back({"shd_closet_dep", 12});
    data.sceneData.push_back({"brt_msk", 13});
    data.sceneData.push_back({"god_ray", 14});// 分隔线

    data.onRenderModeChanged = [&scene](int mode)
    { scene.setRenderFlag(mode); };

    return data;
}