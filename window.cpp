#include "Window.h"
#include <stdio.h> // For snprintf
#include <stdlib.h> // For exit

// Window 构造函数
Window::Window(const char* title, int width, int height)
    // 初始化成员变量
    : scene_()
{
    // ----------------------------------------------------
    // SDL 初始化
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) < 0) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        exit(-1);
    }

    // ----------------------------------------------------
    // OpenGL 属性设置
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);       // 启用双缓冲
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);         // 设置深度缓冲区大小
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);        // 设置模板缓冲区大小
    // 推荐使用，但如果遇到旧驱动问题可以移除
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

    // ----------------------------------------------------
    // 创建 SDL 窗口
    window_ = SDL_CreateWindow(title,
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               width,
                               height,
                               SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window_) {
        printf("Failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        exit(-1);
    }

    // ----------------------------------------------------
    // 创建 OpenGL 上下文
    glContext_ = SDL_GL_CreateContext(window_);
    if (!glContext_) {
        printf("Failed to create OpenGL context: %s\n", SDL_GetError());
        SDL_DestroyWindow(window_);
        SDL_Quit();
        exit(-1);
    }

    // 设置当前 OpenGL 上下文
    if (SDL_GL_MakeCurrent(window_, glContext_) != 0) {
        std::cerr << "Failed to make current context: " << SDL_GetError() << std::endl;
        SDL_GL_DeleteContext(glContext_);
        SDL_DestroyWindow(window_);
        exit(-1);
    }

    // ----------------------------------------------------
    // 使用 GLAD 加载 OpenGL 函数
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        SDL_GL_DeleteContext(glContext_);
        SDL_DestroyWindow(window_);
        SDL_Quit();
        exit(-1);
    }

    // 设置 OpenGL 视口
    int w, h;
    SDL_GL_GetDrawableSize(window_, &w, &h); // 获取实际可绘制区域大小（考虑高DPI）
    glViewport(0, 0, w, h);

    // ----------------------------------------------------
    // V-Sync On/Off (0: Off, 1: On)
    SDL_GL_SetSwapInterval(0); // 禁用 V-Sync，通常在开发阶段更方便

    // ----------------------------------------------------
    // 初始化 UiSystem
    uiSystem_ = new UiSystem(window_, glContext_);

    // 绑定 UiSystem 的回调函数到 Scene
    uiSystem_->onCaptureButtonClicked = [this]() {
        this->scene_.saveTextures(); // 调用 Scene 的 saveTextures 方法
    };

    uiSystem_->uiSceneData = UiSceneData::create(this->scene_); // 假设 UiSceneData::create 方法存在

    uiSystem_->onSsrWeightBarChanged = [this](float weight) {
        this->scene_.setSsrWeight(weight); 
    };
    uiSystem_->ssrWeight = this->scene_.ssrWeight_;

    uiSystem_->onIblWeightBarChanged = [this](float weight) {
        this->scene_.setIblWeight(weight);
    };
    uiSystem_->iblWeight = this->scene_.iblWeight_;

    uiSystem_->onLightWeightBarChanged = [this](float weight) {
        this->scene_.setLightWeight(weight);
    };
    uiSystem_->lightWeight = this->scene_.lightWeight_;

    uiSystem_->onOitWeightBarChanged = [this](float weight) {
        this->scene_.setOitWeight(weight); 
    };
    uiSystem_->oitWeight = this->scene_.oitWeight_;

    uiSystem_->onGodRayWeightBarChanged = [this](float weight) {
        this->scene_.setGodRayWeight(weight); 
    };
    uiSystem_->godRayWeight = this->scene_.godRayWeight_;

    uiSystem_->onFocusDistanceBarChanged = [this](float distance) {
        this->scene_.setFocusDistance(distance); 
    };
    uiSystem_->focusDistance = this->scene_.focusDistance_;

    // ----------------------------------------------------
    // 其他初始化
    lastFPSTime_ = std::chrono::high_resolution_clock::now();
    lastFrameTime_ = std::chrono::high_resolution_clock::now(); // 初始化 deltaTime 的时间点

    scene_.init(); // 初始化你的场景

    // 启用相对鼠标模式，隐藏鼠标光标并将其锁定在窗口中心
    // SDL_SetRelativeMouseMode(SDL_TRUE);
    // SDL_ShowCursor(SDL_FALSE);
}

// Window 析构函数
Window::~Window()
{
    // 清理 UiSystem
    delete uiSystem_;
    uiSystem_ = nullptr;

    // ----------------------------------------------------
    // SDL 清理
    SDL_GL_DeleteContext(glContext_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

// 更新 FPS 计数器和窗口标题
void Window::updateFPS()
{
    // 使用 lastFPSTime_ 来计算 FPS，与 deltaTime_ 分开管理
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsedTime = currentTime - lastFPSTime_;

    frameCount_++;
    if (elapsedTime.count() >= 1.0f) {
        fps_ = frameCount_;
        frameCount_ = 0;
        lastFPSTime_ = currentTime;

        char title[128];
        snprintf(title, sizeof(title), "OpenGL + SDL2 + ImGui - FPS: %d", fps_);
        SDL_SetWindowTitle(window_, title);
    }
}

// 处理所有输入、更新游戏逻辑
void Window::update() {
    // --- 计算 deltaTime ---
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> deltaTimeDuration = currentTime - lastFrameTime_;
    deltaTime_ = deltaTimeDuration.count(); // 获取秒数
    lastFrameTime_ = currentTime;

    // --- 输入处理 ---
    // 先更新 InputManager 的“上一帧”状态，并重置鼠标相对移动量
    InputManager::GetInstance().Update(); 
    
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // 将事件传递给 UI 系统 (如 ImGui)
        uiSystem_->ProcessEvent(&event); 
        // 将事件传递给 InputManager 处理（例如鼠标按钮、滚轮等）
        InputManager::GetInstance().ProcessEvent(event);

        // 处理 SDL_QUIT 事件 (用户点击关闭按钮)
        if (event.type == SDL_QUIT) {
            running_ = false; // 设置内部标志为 false，退出主循环
        }
        // 处理窗口大小变化事件，并传递给 Scene 和 Camera
        if (event.type == SDL_WINDOWEVENT) {
            if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
                event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                int w, h;
                SDL_GetWindowSize(window_, &w, &h);
                // 更新 OpenGL 视口
                glViewport(0, 0, w, h);
                // 通知 Scene 大小改变，以便它可以更新帧缓冲区等
                scene_.resize(w, h);
                // 更新摄像机的宽高比
                // camera_.setAspectRatio(static_cast<float>(w) / h);
            }
        }
    }

    // 检查是否按下了 ESC 键来退出程序
    if (InputManager::GetInstance().IsKeyPressed(SDL_SCANCODE_ESCAPE)) {
        running_ = false;
    }

    // --- 摄像机移动逻辑 ---
    // 使用 InputManager 查询连续按键状态，控制摄像机移动
    // if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_W)) camera_.ProcessKeyboard(FORWARD, deltaTime_);
    // if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_S)) camera_.ProcessKeyboard(BACKWARD, deltaTime_);
    // if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_A)) camera_.ProcessKeyboard(LEFT, deltaTime_);
    // if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_D)) camera_.ProcessKeyboard(RIGHT, deltaTime_);
    // if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_Q)) camera_.ProcessKeyboard(UP, deltaTime_);   // 示例：向上移动
    // if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_E)) camera_.ProcessKeyboard(DOWN, deltaTime_); // 示例：向下移动

    // 使用 InputManager 获取鼠标相对移动量，控制摄像机视角
    // camera_.ProcessMouseMovement(InputManager::GetInstance().GetMouseDeltaX(), InputManager::GetInstance().GetMouseDeltaY());

    // 使用 InputManager 获取鼠标滚轮滚动量，例如控制摄像机 FOV 或移动速度
    // if (InputManager::GetInstance().GetMouseScrollY() != 0) {
    //     camera_.ProcessMouseScroll(InputManager::GetInstance().GetMouseScrollY());
    // }

    // 示例：按下 F1 键切换调试模式 (一次性事件)
    if (InputManager::GetInstance().IsKeyPressed(SDL_SCANCODE_F1)) {
        std::cout << "F1 Pressed! Toggling debug mode..." << std::endl;
        // 未来这里可以执行一个命令，如 new ToggleDebugModeCommand().Execute();
    }

    // 示例：按下鼠标左键执行某个动作 (一次性事件)
    if (InputManager::GetInstance().IsMouseButtonPressed(SDL_BUTTON_LEFT)) {
        std::cout << "Left mouse button clicked!" << std::endl;
        // 例如：new ShootCommand(player).Execute();
    }
    
    // --- 更新 FPS ---
    updateFPS();
}

// 渲染场景和 UI
void Window::render()
{
    // 1. ImGui 新帧
    uiSystem_->NewFrame();

    // 2. 绘制自定义 UI (传入当前 FPS)
    uiSystem_->DrawUI(fps_); // 传入更新后的 fps_

    // 3. 清除缓冲区
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 4. 延迟渲染逻辑：将摄像机的视图和投影矩阵传递给场景
    scene_.run();
    // scene_.run(camera_.GetViewMatrix(), camera_.GetProjectionMatrix());

    // 5. 渲染 ImGui
    uiSystem_->Render();

    // 6. 交换窗口缓冲，显示渲染结果
    SDL_GL_SwapWindow(window_);
}

// 检查窗口是否仍在运行
bool Window::isRunning()
{
    return running_;
}