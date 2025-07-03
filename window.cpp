// Window.cpp (修改后)
#include "window.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "imgui.h"

void LoadRmlUiFonts()
{
    Rml::LoadFontFace("Roboto-Regular.ttf", true, Rml::Style::FontWeight::Normal, 0);
}

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
    // 初始化 ImGui UiSystem
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
    // 初始化 RmlUi
    std::cout << "Initializing RmlUi..." << std::endl;
    rmlUiRenderer_ = new RmlUiOpenGLRenderer();
    if (!rmlUiRenderer_->Initialize())
    {
        std::cerr << "Failed to initialize RmlUi renderer!" << std::endl;
        // 处理错误，可能需要退出
        exit(-1);
    }

    // 假设你的 RML/RCSS 文件在项目根目录下的 "assets/rmlui/" 文件夹中
    // 或者你可以根据实际情况调整这个路径
    rmlUiFileInterface_ = new RmlUiFileInterface("");
    rmlUiSystemInterface_ = new RmlUiSystemInterface();

    // 设置 RmlUi 的接口实例
    Rml::SetRenderInterface(rmlUiRenderer_->GetAdaptedInterface()); // <--- 修正：使用适配器
    Rml::SetSystemInterface(rmlUiSystemInterface_);
    Rml::SetFileInterface(rmlUiFileInterface_);

    // 初始化 RmlUi 核心
    if (!Rml::Initialise())
    { // <--- 修正为 Rml::Initialise
        std::cerr << "Failed to initialise RmlUi Core!" << std::endl;
        exit(-1);
    }

    // 加载字体
    LoadRmlUiFonts();

    // 创建 RmlUi 上下文
    rmlContext_ = Rml::CreateContext("main", Rml::Vector2i(w, h)); // <--- 修正为 Rml::CreateContext
    if (!rmlContext_)
    {
        std::cerr << "Failed to create RmlUi context!" << std::endl;
        exit(-1);
    }
    rmlUiRenderer_->SetViewport(w, h); // <--- 新增这一行

    // 可选：初始化 RmlUi 调试器
    // #ifdef RMLUI_DEBUGGER_ENABLED // 如果你在 CMake 中定义了这个宏
    // Rml::Debugger::Initialise(rmlContext_); // <--- 修正为 Rml::Debugger
    // Rml::Debugger::SetVisible(true); // 默认显示调试器
    // #endif

    // 加载并显示 RML 文档
    rmlDocument_ = rmlContext_->LoadDocument("sample.rml");
    if (rmlDocument_)
    {
        rmlDocument_->Show();
        std::cout << "RmlUi sample.rml loaded and shown." << std::endl;

        // 绑定按钮事件 (例如关闭窗口)
        Rml::Element *closeButton = rmlDocument_->GetElementById("close_button"); // <--- 修正为 Rml::Element
        if (closeButton)
        {
            // closeButton->AddEventListener(Rml::EventId::Click, [this](Rml::Event& event) { // <--- 修正为 Rml::EventId, Rml::Event
            //     this->running_ = false; // 点击按钮关闭窗口
            //     std::cout << "Close button clicked! Exiting application." << std::endl;
            // });
        }
    }
    else
    {
        std::cerr << "Failed to load RmlUi document: sample.rml" << std::endl;
    }

    // ----------------------------------------------------
    // 其他初始化
    lastFPSTime_ = std::chrono::high_resolution_clock::now();
    lastFrameTime_ = std::chrono::high_resolution_clock::now();

    uiSystem_->uiSceneData = UiSceneData::create(scene_);

    //----------------------------------
    commandQueue_ = std::make_unique<CommandQueue>();

    // 1. Create Input Processors and add to the chain (order matters for priority!)
    // High priority first (UI layers)
    auto imguiProc = std::make_unique<ImGuiInputProcessor>();
    imGuiProcessor_ = imguiProc.get(); // Store raw pointer for direct access
    inputProcessors_.push_back(std::move(imguiProc));

    auto rmluiProc = std::make_unique<RmlUiInputProcessor>(rmlContext_); // Pass RmlUi Context
    rmlUiProcessor_ = rmluiProc.get(); // Store raw pointer for direct access
    inputProcessors_.push_back(std::move(rmluiProc));

    // Low priority last (Game layer)
    // Pass necessary dependencies for GameInputProcessor to create commands
    auto gameProc = std::make_unique<GameInputProcessor>(scene_->getCamera(), scene_); // Pass Camera and Scene
    gameInputProcessor_ = gameProc.get(); // Store raw pointer for direct access
    inputProcessors_.push_back(std::move(gameProc));
}

// Window 析构函数
Window::~Window()
{
    // ----------------------------------------------------
    // 清理 RmlUi 资源
    std::cout << "Shutting down RmlUi..." << std::endl;
    if (rmlDocument_)
    {
        rmlDocument_->Close();
        rmlDocument_ = nullptr;
    }
    if (rmlContext_)
    {
        Rml::RemoveContext(rmlContext_->GetName()); // <--- 修正为 Rml::RemoveContext
        rmlContext_ = nullptr;
    }
    Rml::Shutdown(); // <--- 修正为 Rml::Shutdown

    delete rmlUiRenderer_;
    rmlUiRenderer_ = nullptr;
    delete rmlUiSystemInterface_;
    rmlUiSystemInterface_ = nullptr;
    delete rmlUiFileInterface_;
    rmlUiFileInterface_ = nullptr;

    // 清理 ImGui UiSystem 资源
    delete uiSystem_;
    uiSystem_ = nullptr;

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
        snprintf(title, sizeof(title), "OpenGL + SDL2 + ImGui + RmlUi - FPS: %d", fps_); // <--- 更新标题
        SDL_SetWindowTitle(window_, title);

        // 更新 RmlUi 中的 FPS 显示
        if (rmlDocument_)
        {
            Rml::Element *fpsElement = rmlDocument_->GetElementById("fps_display"); // <--- 修正为 Rml::Element
            if (fpsElement)
            {
                fpsElement->SetInnerRML(std::to_string(fps_).c_str());
            }
        }
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

    // --- 输入处理 ---
     // 1. Prepare InputManager for the new frame (clear deltas, update previous states)
    InputManager::GetInstance().Update();

    // 2. Prepare each InputProcessor for the new frame (e.g., ImGui::NewFrame())
    for (const auto& processor : inputProcessors_) {
        processor->BeginFrame();
    }

    SDL_Event event;
    bool uiCapturesKeyboard = false;
    bool uiCapturesMouse = false;
        while (SDL_PollEvent(&event)) {
        // Handle global system events first (like quit request or window resize)
        if (event.type == SDL_QUIT) {
            running_ = false; // Set global flag to stop the main loop
            // No need to pass SDL_QUIT to InputManager::ProcessEvent
            // because InputManager::IsQuitRequested() will handle it via the event directly.
            continue; // Don't pass QUIT to other processors unless they explicitly need it
        }

        // Example: Window resize event (can be handled globally or passed through processors)
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
            // Your window resize logic
            int newWidth = event.window.data1;
            int newHeight = event.window.data2;
            std::cout << "Window resized to: " << newWidth << "x" << newHeight << std::endl;
            // Potentially update RmlUi context size, ImGui display size etc.
            rmlUiRenderer_->SetViewport(newWidth, newHeight); // Assuming RmlUiSystem has this
            // You might also need to update OpenGL viewport, projection matrix etc.
        }

        // Dispatch the event through the chain
        // We iterate in reverse to allow high-priority UI processors to set their capture flags
        // However, a simpler forward iteration is often sufficient if capture checks happen AFTER the loop.
        // Let's stick with forward iteration for ProcessEvent, and gather capture flags later.
        for (const auto& processor : inputProcessors_) {
            // ProcessEvent might add commands to commandQueue_
            if (processor->ProcessEvent(event, *commandQueue_)) {
                // If a processor returns true, it means it "consumed" the event.
                // We typically stop processing for this event down the chain.
                // However, for ImGui/RmlUi, they often return false but set internal WantCapture flags.
                // It depends on how strict you want the "consumption" to be.
                // For now, let's let all processors see all events, but let the GenerateCommands decide based on capture flags.
                // The "return true" is more useful if you have mutually exclusive event handlers.
                // For UI vs Game, the WantsToCapture* flags are the primary mechanism.
            }
        }
    }

    // 4. Gather UI capture states AFTER all events have been processed by UIs
    // This is crucial: ImGui and RmlUi update their WantCapture* flags *during* ProcessEvent.
    // So, we query their final state after the event loop.
    if (imGuiProcessor_) {
        uiCapturesKeyboard = imGuiProcessor_->WantsToCaptureKeyboard();
        uiCapturesMouse = imGuiProcessor_->WantsToCaptureMouse();
    }
    // RmlUi takes priority over ImGui if both capture (you can adjust this logic)
    // Here, if RmlUi wants capture, it overrides ImGui.
    if (rmlUiProcessor_) {
        if (rmlUiProcessor_->WantsToCaptureKeyboard()) {
            uiCapturesKeyboard = true;
        }
        if (rmlUiProcessor_->WantsToCaptureMouse()) {
            uiCapturesMouse = true;
        }
    }

    // 5. Generate Game-specific Commands (only if UI doesn't capture input)
    if (gameInputProcessor_) {
        gameInputProcessor_->GenerateCommands(*commandQueue_, deltaTime_, uiCapturesKeyboard, uiCapturesMouse);
    }

    // 6. Process all commands accumulated in the queue
    commandQueue_->ProcessCommands();

    // --- RmlUi 更新 ---
    if (rmlContext_)
    {
        rmlContext_->Update();
    }

    updateFPS();
}

// 渲染场景和 UI
void Window::render()
{
    uiSystem_->NewFrame();
    uiSystem_->DrawUI(fps_);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    scene_->run(deltaTime_);

    if (rmlContext_)
    {
        rmlContext_->Render();
    }

    uiSystem_->Render();

    // 7. 交换窗口缓冲
    SDL_GL_SwapWindow(window_);
}

// 检查窗口是否仍在运行
bool Window::isRunning()
{
    return running_;
}
