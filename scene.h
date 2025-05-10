#ifndef _SCENE_H_
#define _SCENE_H_

#include <string>
#include "camera.h"
#include "GBufferPass.h"
#include "screenPass.h"
#include "lightPass.h"
#include "SceneData.h"
#include <memory>
#include "mesh.h"

class Scene
{
public:
    void init();
    void run();

private:
    SceneData sceneData_;
    std::unique_ptr<GBufferPass> gBufferPass_;
    std::unique_ptr<ScreenPass> screenPass_;
    std::unique_ptr<LightPass> lightPass_;
    Camera camera_;
};

#endif