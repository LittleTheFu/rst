#include "render.h"

Render::Render(int width, int height) : screenWidth_(width), screenHeight_(height) {
    init();
}

Render::~Render() {
    // 智能指针会自动释放资源
}

void Render::init() {
    // 初始化 Camera
    camera_.Position = Eigen::Vector3f(0.0f, 0.0f, 3.0f); // 示例位置
    camera_.Yaw = -90.0f;
    camera_.Pitch = 0.0f;
    camera_.ProcessMouseMovement(0, 0); // 初始化相机方向

    // 初始化 SceneData (示例)
    sceneData_.screenWidth = screenWidth_;
    sceneData_.screenHeight = screenHeight_;
    //  ... 添加场景中的物体，光源等

    // 初始化 G-Buffer Pass
    gBufferPass_ = std::make_unique<GBufferPass>();
    gBufferPass_->Initialize(screenWidth_, screenHeight_);

    // 初始化其他 RenderPass...
}

void Render::run() {
    // 更新 Camera (例如，基于用户输入)
    // camera_.ProcessInput(window); // 假设你需要传入窗口信息
    camera_.updateCameraVectors();

    // 1. G-Buffer Pass
    gBufferPass_->Render(sceneData_, camera_);

    // 2. 其他 RenderPass (例如，光照 Pass, 最终渲染 Pass)
    // ...

    // 3. 渲染到屏幕
    // ...
}

void Render::resize(int width, int height) {
    screenWidth_ = width;
    screenHeight_ = height;
    gBufferPass_->Resize(width, height);
    sceneData_.screenWidth = width;
    sceneData_.screenHeight = height;

    //  ... resize 其他 RenderPass
}