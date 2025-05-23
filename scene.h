#ifndef _SCENE_H_
#define _SCENE_H_

#include <string>
#include "camera.h"
#include "GBufferPass.h"
#include "lightPass.h"
#include "SceneData.h"
#include <memory>
#include "mesh.h"
#include "screenPass.h"
#include "skyPass.h"
#include "shadowPass.h"
#include "IBLPass.h"
#include "TextureCubeMap.h"
#include "Texture2D.h" // 新增：包含 Texture2D

class Scene
{
public:
    void init();
    void run();

private:
    SceneData sceneData_;

    std::unique_ptr<GBufferPass> gBufferPass_;
    std::unique_ptr<LightPass> lightPass_;
    std::unique_ptr<SkyPass> skyPass_;
    std::unique_ptr<ScreenPass> screenPass_;
    std::unique_ptr<ShadowPass> shadowPass_;
    std::unique_ptr<IBLPass> iblPass_;

    std::shared_ptr<TextureCubeMap> irradianceMapTex_;
    std::shared_ptr<TextureCubeMap> prefilterMapTex_;
    std::shared_ptr<Texture2D> brdfLUTTex_; // 修改类型为 Texture2D

    Camera camera_;
    Camera shadow_camera_;
};

#endif // _SCENE_H_