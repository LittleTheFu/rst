#ifndef _SCENE_H_
#define _SCENE_H_

#include <string>
#include "camera.h"
#include "GBufferPass.h"
#include "SceneData.h"
#include <memory>

class Scene
{
public:
    void init();
    void run();

private:
    SceneData sceneData_;
    std::unique_ptr<GBufferPass> gBufferPass_;
    Camera camera_;
    std::string vertex_shader_;
    std::string fragment_shader_;
    unsigned int shaderProgram_;
    unsigned int VAO_;
    unsigned int VBO_;
    std::unique_ptr<Renderable> createTriangleMesh(); // 声明
};

#endif