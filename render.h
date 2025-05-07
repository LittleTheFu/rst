#ifndef _RENDER_H_
#define _RENDER_H_

#include <memory>
#include <vector>
#include "GBufferPass.h"
#include "Camera.h" // 假设你有一个 Camera 类
#include "SceneData.h" // 假设你有一个 SceneData 结构体

class Render {
public:
    Render(int width, int height);
    ~Render();

    void init();
    void run();
    void resize(int width, int height);

private:
    int screenWidth_;
    int screenHeight_;
    std::unique_ptr<GBufferPass> gBufferPass_;
    Camera camera_;
    SceneData sceneData_;
    // 其他 RenderPass 对象...
};

#endif