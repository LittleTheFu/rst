#ifndef _SCENE_H_
#define _SCENE_H_

#include <string>
#include "camera.h"
#include "GBufferPass.h"
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
    Camera camera_;
};

#endif