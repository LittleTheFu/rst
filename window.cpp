#include "window.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "imgui.h" // 确保包含 ImGui 头文件，因为使用了 ImGui::GetIO().WantCaptureMouse

// Window 构造函数
Window::Window(const char *title, int width, int height)
{
    // ----------------------------------------------------
    // SDL 初始化
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) < 0)
    {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        exit(-1);
    }

    // ----------------------------------------------------
    // OpenGL 属性设置
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

    // ----------------------------------------------------
    // 创建 SDL 窗口
    window_ = SDL_CreateWindow(title,
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               width,
                               height,
                               SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window_)
    {
        printf("Failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        exit(-1);
    }

    // ----------------------------------------------------
    // 创建 OpenGL 上下文
    glContext_ = SDL_GL_CreateContext(window_);
    if (!glContext_)
    {
        printf("Failed to create OpenGL context: %s\n", SDL_GetError());
        SDL_DestroyWindow(window_);
        SDL_Quit();
        exit(-1);
    }

    // 设置当前 OpenGL 上下文为刚刚创建的上下文
    if (SDL_GL_MakeCurrent(window_, glContext_) != 0)
    {
        std::cerr << "Failed to make current context: " << SDL_GetError() << std::endl;
        SDL_GL_DeleteContext(glContext_);
        SDL_DestroyWindow(window_);
        exit(-1);
    }

    // ----------------------------------------------------
    // 使用 GLAD 加载 OpenGL 函数指针
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        printf("Failed to initialize GLAD\n");
        SDL_GL_DeleteContext(glContext_);
        SDL_DestroyWindow(window_);
        SDL_Quit();
        exit(-1);
    }

    // 设置 OpenGL 视口
    int w, h;
    SDL_GL_GetDrawableSize(window_, &w, &h);
    glViewport(0, 0, w, h);

    // ----------------------------------------------------
    // V-Sync On/Off (0: Off, 1: On)
    SDL_GL_SetSwapInterval(0); // 禁用 V-Sync


    // init scene first
    scene_ = std::make_shared<Scene>();
    scene_->init();
    
    // ----------------------------------------------------
    // 初始化 UiSystem
    uiSystem_ = new UiSystem(window_, glContext_);

    // 绑定 UiSystem 的回调函数到 Scene
    uiSystem_->onCaptureButtonClicked = [this]()
    {
        scene_->saveTextures();
    };

    uiSystem_->onToggleDebugButtonClicked = [this]()
    {
        scene_->toggleDebugDraw();
    };

    // 绑定各种 UI 滑块的回调函数，以调整场景参数
    uiSystem_->onSsrWeightBarChanged = [this](float weight)
    {
        scene_->setSsrWeight(weight);
    };
    uiSystem_->ssrWeight = scene_->ssrWeight_;

    uiSystem_->onIblWeightBarChanged = [this](float weight)
    {
        scene_->setIblWeight(weight);
    };
    uiSystem_->iblWeight = scene_->iblWeight_;

    uiSystem_->onLightWeightBarChanged = [this](float weight)
    {
        scene_->setLightWeight(weight);
    };
    uiSystem_->lightWeight = scene_->lightWeight_;

    uiSystem_->onOitWeightBarChanged = [this](float weight)
    {
        scene_->setOitWeight(weight);
    };
    uiSystem_->oitWeight = scene_->oitWeight_;

    uiSystem_->onGodRayWeightBarChanged = [this](float weight)
    {
        scene_->setGodRayWeight(weight);
    };
    uiSystem_->godRayWeight = scene_->godRayWeight_;

    uiSystem_->onFocusDistanceBarChanged = [this](float distance)
    {
        scene_->setFocusDistance(distance);
    };
    uiSystem_->focusDistance = scene_->focusDistance_;

    // !!! 关键改动 !!!
    // onGetSelectedMesh 更改为 onGetSelectedObject，并返回 ISceneObject*
    uiSystem_->onGetSelectedObject = [this]()
    {
        return scene_->getSelectedObject();
    };

    // ----------------------------------------------------
    // 其他初始化
    lastFPSTime_ = std::chrono::high_resolution_clock::now();
    lastFrameTime_ = std::chrono::high_resolution_clock::now();

    // 假设 UiSceneData::create 方法存在，用于从 Scene 初始化 UI 数据
    // 确保 UiSceneData 也能处理 ISceneObject
    uiSystem_->uiSceneData = UiSceneData::create(scene_);

    // --- 初始化命令对象 ---
    // CameraCommand 的 setDeltaTime 方法会在 update() 中每帧调用
    cmd_moveForward_ = new MoveCameraForwardCommand(scene_->getCamera());
    cmd_moveBackward_ = new MoveCameraBackwardCommand(scene_->getCamera());
    cmd_moveLeft_ = new MoveCameraLeftCommand(scene_->getCamera());
    cmd_moveRight_ = new MoveCameraRightCommand(scene_->getCamera());
    cmd_moveUp_ = new MoveCameraUpCommand(scene_->getCamera());
    cmd_moveDown_ = new MoveCameraDownCommand(scene_->getCamera());
    cmd_rotateLeft_ = new RotateCameraLeftCommand(scene_->getCamera());
    cmd_rotateRight_ = new RotateCameraRightCommand(scene_->getCamera());

    cmd_mouseLook_ = new ProcessMouseMovementCommand(scene_->getCamera());
    cmd_mouseScroll_ = new ProcessMouseScrollCommand(scene_->getCamera());

    // 传递 Scene* 给 ToggleDebugModeCommand
    cmd_toggleDebug_ = new ToggleDebugModeCommand(scene_);

    // 传递 ObjectPicker* 给 PickObjectCommand
    cmd_pickObject_ = new PickObjectCommand(scene_->objectPicker_.get());
}

// Window 析构函数
Window::~Window()
{
    // 清理 UiSystem 资源
    delete uiSystem_;
    uiSystem_ = nullptr;

    // --- 清理命令对象 ---
    delete cmd_moveForward_;
    cmd_moveForward_ = nullptr;
    delete cmd_moveBackward_;
    cmd_moveBackward_ = nullptr;
    delete cmd_moveLeft_;
    cmd_moveLeft_ = nullptr;
    delete cmd_moveRight_;
    cmd_moveRight_ = nullptr;
    delete cmd_moveUp_;
    cmd_moveUp_ = nullptr;
    delete cmd_moveDown_;
    cmd_moveDown_ = nullptr;
    delete cmd_rotateLeft_;
    cmd_rotateLeft_ = nullptr;
    delete cmd_rotateRight_;
    cmd_rotateRight_ = nullptr;
    delete cmd_mouseLook_;
    cmd_mouseLook_ = nullptr;
    delete cmd_mouseScroll_;
    cmd_mouseScroll_ = nullptr;
    delete cmd_toggleDebug_;
    cmd_toggleDebug_ = nullptr;
    delete cmd_pickObject_;
    cmd_pickObject_ = nullptr;

    // ----------------------------------------------------
    // SDL 和 OpenGL 资源清理
    SDL_GL_DeleteContext(glContext_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

// 更新 FPS 计数器和窗口标题
void Window::updateFPS()
{
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsedTime = currentTime - lastFPSTime_;

    frameCount_++;
    if (elapsedTime.count() >= 1.0f)
    {
        fps_ = frameCount_;
        frameCount_ = 0;
        lastFPSTime_ = currentTime;

        char title[128];
        snprintf(title, sizeof(title), "OpenGL + SDL2 + ImGui - FPS: %d", fps_);
        SDL_SetWindowTitle(window_, title);
    }
}

// 处理所有输入、更新游戏逻辑
void Window::update()
{
    // --- 计算 deltaTime ---
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> deltaTimeDuration = currentTime - lastFrameTime_;
    deltaTime_ = deltaTimeDuration.count();
    lastFrameTime_ = currentTime;

    // --- 更新命令的 deltaTime (针对需要它的命令) ---
    // 假设 CameraCommand 基类有 setDeltaTime 方法
    if (cmd_moveForward_)
        cmd_moveForward_->setDeltaTime(deltaTime_);
    if (cmd_moveBackward_)
        cmd_moveBackward_->setDeltaTime(deltaTime_);
    if (cmd_moveLeft_)
        cmd_moveLeft_->setDeltaTime(deltaTime_);
    if (cmd_moveRight_)
        cmd_moveRight_->setDeltaTime(deltaTime_);
    if (cmd_moveUp_)
        cmd_moveUp_->setDeltaTime(deltaTime_);
    if (cmd_moveDown_)
        cmd_moveDown_->setDeltaTime(deltaTime_);
    if (cmd_rotateLeft_)
        cmd_rotateLeft_->setDeltaTime(deltaTime_);
    if (cmd_rotateRight_)
        cmd_rotateRight_->setDeltaTime(deltaTime_);

    // --- 输入处理 ---
    InputManager::GetInstance().Update();

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        uiSystem_->ProcessEvent(&event);
        InputManager::GetInstance().ProcessEvent(event);

        if (event.type == SDL_QUIT)
        {
            running_ = false;
        }
        if (event.type == SDL_WINDOWEVENT)
        {
            if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
                event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                int w, h;
                SDL_GetWindowSize(window_, &w, &h);
                glViewport(0, 0, w, h);
                scene_->resize(w, h);
                scene_->getCamera()->setAspectRatio(static_cast<float>(w) / h);
            }
        }
    }

    // --- 命令调度 (Invoke Commands) ---
    if (InputManager::GetInstance().IsKeyPressed(SDL_SCANCODE_ESCAPE))
    {
        running_ = false;
    }

    if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_W))
        cmd_moveForward_->Execute();
    if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_S))
        cmd_moveBackward_->Execute();
    if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_A))
        cmd_moveLeft_->Execute();
    if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_D))
        cmd_moveRight_->Execute();
    if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_Q))
        cmd_moveUp_->Execute();
    if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_E))
        cmd_moveDown_->Execute();
    if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_Z))
        cmd_rotateLeft_->Execute();
    if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_C))
        cmd_rotateRight_->Execute();

    cmd_mouseLook_->Execute();
    cmd_mouseScroll_->Execute();

    if (InputManager::GetInstance().IsKeyPressed(SDL_SCANCODE_F1))
    {
        cmd_toggleDebug_->Execute();
    }

    if (InputManager::GetInstance().IsMouseButtonPressed(SDL_BUTTON_LEFT))
    {
        // 核心解决方案：检查 ImGui 是否想捕获鼠标
        if (!ImGui::GetIO().WantCaptureMouse)
        {
            cmd_pickObject_->setMousePosition(InputManager::GetInstance().GetMouseX(), InputManager::GetInstance().GetMouseY());
            cmd_pickObject_->Execute();

            // !!! 关键改动 !!!
            // 拾取结果现在是 ISceneObject*
            pickedObject_ = cmd_pickObject_->getPickedObject(); // 从命令中获取拾取到的对象

            if (pickedObject_)
            {
                scene_->setSelectedObject(pickedObject_); // 通知 Scene 选中了哪个对象
                std::cout << "Picked object: " << pickedObject_->getName() << std::endl;
            }
            else
            {
                scene_->setSelectedObject(nullptr); // 如果没选中任何东西，则清空选中状态
                std::cout << "No object picked." << std::endl;
            }
        }
        else
        {
            // 当 ImGui 捕获鼠标时，如果需要，仍然可以打印上一个选中对象的信息
            if (pickedObject_)
            {
                std::cout << "ImGui capturing mouse. Last picked object: " << pickedObject_->getName() << std::endl;
            }
            else
            {
                std::cout << "ImGui capturing mouse. No object picked." << std::endl;
            }
        }
    }

    // --- 更新 FPS ---
    updateFPS();
}

// 渲染场景和 UI
void Window::render()
{
    // 1. 开始 ImGui 新帧的绘制
    uiSystem_->NewFrame();

    // 2. 绘制自定义 UI 界面
    // uiSystem_->DrawUI 内部会调用 scene_->getSelectedObjectName()
    uiSystem_->DrawUI(fps_);

    // 3. 清除颜色缓冲区和深度缓冲区
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 4. 执行场景的渲染逻辑
    scene_->run();

    // 5. 渲染 ImGui 界面到 OpenGL
    uiSystem_->Render();

    // 6. 交换窗口缓冲
    SDL_GL_SwapWindow(window_);
}

// 检查窗口是否仍在运行
bool Window::isRunning()
{
    return running_;
}