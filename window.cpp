#include "Window.h"
#include <stdio.h>  // For snprintf
#include <stdlib.h> // For exit
#include <iostream> // For std::cerr, std::cout

// Window 构造函数
Window::Window(const char* title, int width, int height)
    // 成员初始化列表：
    // 初始化 scene_ (默认构造函数)
    // 初始化 camera_，传入初始位置、世界向上向量、偏航角、俯仰角以及初始宽高比
    : scene_(), 
      camera_()
{
    // ----------------------------------------------------
    // SDL 初始化
    // 初始化视频、定时器和游戏手柄子系统，ImGui 等可能需要。
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) < 0) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        exit(-1);
    }

    // ----------------------------------------------------
    // OpenGL 属性设置
    // 设置 OpenGL 上下文版本为 4.6 核心模式
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);       // 启用双缓冲
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);         // 设置深度缓冲区大小
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);        // 设置模板缓冲区大小
    // 启用前向兼容性，推荐在现代 OpenGL 中使用
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

    // ----------------------------------------------------
    // 创建 SDL 窗口
    window_ = SDL_CreateWindow(title,
                               SDL_WINDOWPOS_CENTERED, // 窗口居中
                               SDL_WINDOWPOS_CENTERED,
                               width,
                               height,
                               // 开启 OpenGL 支持，窗口可见，可调整大小，支持高 DPI 屏幕
                               SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window_) {
        printf("Failed to create window: %s\n", SDL_GetError());
        SDL_Quit(); // 窗口创建失败，退出 SDL
        exit(-1);
    }

    // ----------------------------------------------------
    // 创建 OpenGL 上下文
    glContext_ = SDL_GL_CreateContext(window_);
    if (!glContext_) {
        printf("Failed to create OpenGL context: %s\n", SDL_GetError());
        SDL_DestroyWindow(window_); // 上下文创建失败，销毁窗口
        SDL_Quit();
        exit(-1);
    }

    // 设置当前 OpenGL 上下文为刚刚创建的上下文
    if (SDL_GL_MakeCurrent(window_, glContext_) != 0) {
        std::cerr << "Failed to make current context: " << SDL_GetError() << std::endl;
        SDL_GL_DeleteContext(glContext_); // 设置失败，清理上下文
        SDL_DestroyWindow(window_);
        exit(-1);
    }

    // ----------------------------------------------------
    // 使用 GLAD 加载 OpenGL 函数指针
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        SDL_GL_DeleteContext(glContext_);
        SDL_DestroyWindow(window_);
        SDL_Quit();
        exit(-1);
    }

    // 设置 OpenGL 视口
    int w, h;
    // 获取实际可绘制区域大小，这在处理高 DPI 屏幕时很重要
    SDL_GL_GetDrawableSize(window_, &w, &h); 
    glViewport(0, 0, w, h);

    // ----------------------------------------------------
    // V-Sync On/Off (0: Off, 1: On)
    // 0 表示禁用垂直同步 (无限制帧率)，1 表示启用垂直同步 (帧率限制为显示器刷新率)
    SDL_GL_SetSwapInterval(0); // 禁用 V-Sync，通常在开发阶段更方便

    // ----------------------------------------------------
    // 初始化 UiSystem
    uiSystem_ = new UiSystem(window_, glContext_);

    // 绑定 UiSystem 的回调函数到 Scene
    uiSystem_->onCaptureButtonClicked = [this]() {
        this->scene_.saveTextures(); // 调用 Scene 的 saveTextures 方法
    };

    uiSystem_->onToggleDebugButtonClicked = [this]() {
        this->scene_.toggleDebugDraw(); // 调用 Scene 的 toggleDebugDraw 方法
    };

    // 假设 UiSceneData::create 方法存在，用于从 Scene 初始化 UI 数据
    uiSystem_->uiSceneData = UiSceneData::create(this->scene_); 

    // 绑定各种 UI 滑块的回调函数，以调整场景参数
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
    lastFPSTime_ = std::chrono::high_resolution_clock::now();   // 初始化 FPS 计时器
    lastFrameTime_ = std::chrono::high_resolution_clock::now(); // 初始化 deltaTime 计时器

    scene_.init(); // 初始化你的场景（例如加载模型、着色器等）


    // --- 初始化命令对象 ---
    // 这里创建具体的命令实例，并将 camera_ 实例的地址作为接收者传递给它们。
    // deltaTime_ 是 Window 的成员变量，其值会在 update() 方法的开始处更新。
    // CameraCommand 基类中的 deltaTime_ 字段在每帧更新时会被同步。
    cmd_moveForward_ = new MoveCameraForwardCommand(&camera_, deltaTime_);
    cmd_moveBackward_ = new MoveCameraBackwardCommand(&camera_, deltaTime_);
    cmd_moveLeft_ = new MoveCameraLeftCommand(&camera_, deltaTime_);
    cmd_moveRight_ = new MoveCameraRightCommand(&camera_, deltaTime_);
    cmd_moveUp_ = new MoveCameraUpCommand(&camera_, deltaTime_);
    cmd_moveDown_ = new MoveCameraDownCommand(&camera_, deltaTime_);
    
    // 鼠标视角和滚轮命令不需要直接接收 deltaTime，因为它们从 InputManager 获取相对量
    cmd_mouseLook_ = new ProcessMouseMovementCommand(&camera_);
    cmd_mouseScroll_ = new ProcessMouseScrollCommand(&camera_);
    
    // 调试模式切换命令，不依赖 Camera
    cmd_toggleDebug_ = new ToggleDebugModeCommand(); 
}

// Window 析构函数
Window::~Window()
{
    // 清理 UiSystem 资源
    delete uiSystem_;
    uiSystem_ = nullptr;

    // --- 清理命令对象 ---
    // 释放所有在堆上分配的命令对象，避免内存泄漏
    delete cmd_moveForward_;
    delete cmd_moveBackward_;
    delete cmd_moveLeft_;
    delete cmd_moveRight_;
    delete cmd_moveUp_;
    delete cmd_moveDown_;
    delete cmd_mouseLook_;
    delete cmd_mouseScroll_;
    delete cmd_toggleDebug_;

    // ----------------------------------------------------
    // SDL 和 OpenGL 资源清理
    SDL_GL_DeleteContext(glContext_); // 销毁 OpenGL 上下文
    SDL_DestroyWindow(window_);       // 销毁 SDL 窗口
    SDL_Quit();                       // 退出 SDL 子系统
}

// 更新 FPS 计数器和窗口标题
void Window::updateFPS()
{
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsedTime = currentTime - lastFPSTime_;

    frameCount_++; // 增加帧计数
    if (elapsedTime.count() >= 1.0f) { // 如果自上次更新已超过 1 秒
        fps_ = frameCount_;             // 更新 FPS 值
        frameCount_ = 0;                // 重置帧计数
        lastFPSTime_ = currentTime;     // 重置计时器

        char title[128];
        // 更新窗口标题，显示当前 FPS
        snprintf(title, sizeof(title), "OpenGL + SDL2 + ImGui - FPS: %d", fps_);
        SDL_SetWindowTitle(window_, title);
    }
}

// 处理所有输入、更新游戏逻辑
void Window::update() {
    // --- 计算 deltaTime ---
    // 获取当前帧的时间点，并计算与上一帧之间的时间差（秒）
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> deltaTimeDuration = currentTime - lastFrameTime_;
    deltaTime_ = deltaTimeDuration.count(); 
    lastFrameTime_ = currentTime;

    // --- 更新命令的 deltaTime (针对需要它的命令) ---
    // 由于 deltaTime_ 是每帧变化的，需要确保传递给命令的值是最新的。
    // 这里通过 static_cast 访问 CameraCommand 的 protected 成员 deltaTime_ 进行更新。
    if (cmd_moveForward_) static_cast<CameraCommand*>(cmd_moveForward_)->setDeltaTime(deltaTime_);
    if (cmd_moveBackward_) static_cast<CameraCommand*>(cmd_moveBackward_)->setDeltaTime(deltaTime_);
    if (cmd_moveLeft_) static_cast<CameraCommand*>(cmd_moveLeft_)->setDeltaTime(deltaTime_);
    if (cmd_moveRight_) static_cast<CameraCommand*>(cmd_moveRight_)->setDeltaTime(deltaTime_);
    if (cmd_moveUp_) static_cast<CameraCommand*>(cmd_moveUp_)->setDeltaTime(deltaTime_);
    if (cmd_moveDown_) static_cast<CameraCommand*>(cmd_moveDown_)->setDeltaTime(deltaTime_);

    // --- 输入处理 ---
    // 1. 调用 InputManager::Update() 清理上一帧状态，并重置鼠标相对移动量。
    InputManager::GetInstance().Update(); 
    
    // 2. 轮询所有 SDL 事件，并将每个事件传递给相应的系统。
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        uiSystem_->ProcessEvent(&event);      // 将事件传递给 UI 系统 (如 ImGui)
        InputManager::GetInstance().ProcessEvent(event); // 将事件传递给 InputManager (用于更新鼠标状态等)

        // 处理 SDL_QUIT 事件 (用户点击窗口关闭按钮)
        if (event.type == SDL_QUIT) {
            running_ = false; // 设置内部标志为 false，退出主循环
        }
        // 处理窗口大小变化事件
        if (event.type == SDL_WINDOWEVENT) {
            if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
                event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                int w, h;
                SDL_GetWindowSize(window_, &w, &h); // 获取新的窗口尺寸
                glViewport(0, 0, w, h);             // 更新 OpenGL 视口
                scene_.resize(w, h);                // 通知 Scene 调整其内部资源（如帧缓冲区）
                camera_.setAspectRatio(static_cast<float>(w) / h); // 更新摄像机的宽高比
            }
        }
    }

    // --- 命令调度 (Invoke Commands) ---
    // 根据 InputManager 的查询结果，决定执行哪些命令。
    // 这是输入逻辑的核心，将抽象的输入与具体的行为解耦。

    // 退出程序命令：按下 ESC 键
    if (InputManager::GetInstance().IsKeyPressed(SDL_SCANCODE_ESCAPE)) {
        running_ = false; 
    }

    // 键盘移动命令：检查按键是否被持续按下，然后执行对应的摄像机移动命令
    if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_W)) {
        cmd_moveForward_->Execute();
    }
    if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_S)) {
        cmd_moveBackward_->Execute();
    }
    if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_A)) {
        cmd_moveLeft_->Execute();
    }
    if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_D)) {
        cmd_moveRight_->Execute();
    }
    if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_Q)) {
        cmd_moveUp_->Execute();
    }
    if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_E)) {
        cmd_moveDown_->Execute();
    }

    // 鼠标视角命令：总是执行，因为即使鼠标没有移动，GetMouseDeltaX/Y 也会被重置为 0，
    // 命令内部会检查是否实际有位移。
    cmd_mouseLook_->Execute();

    // 鼠标滚轮命令：类似鼠标视角命令，命令内部会检查 GetMouseScrollY 是否为 0。
    cmd_mouseScroll_->Execute();

    // 示例：F1 键切换调试模式 (一次性按键触发)
    if (InputManager::GetInstance().IsKeyPressed(SDL_SCANCODE_F1)) {
        cmd_toggleDebug_->Execute();
    }

    // 示例：鼠标左键点击 (一次性点击触发)
    if (InputManager::GetInstance().IsMouseButtonPressed(SDL_BUTTON_LEFT)) {
        std::cout << "Left mouse button clicked (via command)!" << std::endl;
        // 可以在这里执行一个射击命令、交互命令等
    }
    
    // --- 更新 FPS ---
    updateFPS();
}

// 渲染场景和 UI
void Window::render()
{
    // 1. 开始 ImGui 新帧的绘制
    uiSystem_->NewFrame();

    // 2. 绘制自定义 UI 界面（例如调试窗口、性能数据显示等）
    uiSystem_->DrawUI(fps_); // 传入当前帧率以便 UI 显示

    // 3. 清除颜色缓冲区和深度缓冲区
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // 设置背景颜色为深灰色
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清除颜色和深度信息

    // 4. 执行场景的渲染逻辑，并将摄像机的视图和投影矩阵传递给场景
    // 场景将使用这些矩阵来正确地渲染物体
    // scene_.run(camera_.GetViewMatrix(), camera_.GetProjectionMatrix());
    scene_.run();

    // 5. 渲染 ImGui 界面到 OpenGL
    uiSystem_->Render();

    // 6. 交换窗口缓冲，将渲染好的图像显示到屏幕上 (双缓冲机制)
    SDL_GL_SwapWindow(window_);
}

// 检查窗口是否仍在运行
// 它返回内部的 running_ 标志，该标志由 update() 中的事件处理更新。
bool Window::isRunning()
{
    return running_;
}