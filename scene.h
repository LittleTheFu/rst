#ifndef _SCENE_H_
#define _SCENE_H_

#include <string>
#include "camera.h"
#include "GBufferPass.h"
#include "lightPass.h"
#include "lightPass.h"
#include "SceneData.h"
#include <memory>
#include "mesh.h"
#include "screenPass.h"
#include "skyPass.h"
#include "shadowPass.h"

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
    
    Camera camera_;
    Camera shadow_camera_;
};

#endif