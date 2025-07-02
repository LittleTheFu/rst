// Window.cpp (修改后)
#include "window.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "imgui.h" // 确保包含 ImGui 头文件，因为使用了 ImGui::GetIO().WantCaptureMouse

// RmlUi 字体加载辅助函数
// 你可能需要根据你的字体文件路径调整
void LoadRmlUiFonts() {
    // 假设你的字体文件在 "assets/fonts/" 目录下
    // NotoSansCJKsc-Regular.otf 是一个常见的支持中文的字体
    // 你需要确保这个字体文件存在于你的项目路径中
    Rml::LoadFontFace("Roboto-Regular.ttf", true, Rml::Style::FontWeight::Normal, 0); 


    
    // 加载粗体 (Bold)
    // Rml::LoadFontFace("NotoSansSC-Bold.ttf", true, Rml::Style::FontWeight::Bold, 0);

    // 可以加载更多字体
    // Rml::FontDatabase::LoadFontFace("assets/fonts/OpenSans-Regular.ttf", "Open Sans");
}

// 辅助函数：将 SDL_Keycode 映射到 Rml::Input::KeyIdentifier
// 这是一个简化的映射，对于所有键，你可能需要一个更完整的 switch-case
Rml::Input::KeyIdentifier SDLKeyToRmlKey(SDL_Keycode sdl_key) {
    using namespace Rml::Input;
    switch (sdl_key) {
        case SDLK_BACKSPACE: return KI_BACK;
        case SDLK_TAB: return KI_TAB;
        case SDLK_CLEAR: return KI_CLEAR;
        case SDLK_RETURN: return KI_RETURN;
        case SDLK_PAUSE: return KI_PAUSE;
        case SDLK_ESCAPE: return KI_ESCAPE;
        case SDLK_SPACE: return KI_SPACE;
        case SDLK_EXCLAIM: return KI_OEM_1; // !
        case SDLK_QUOTEDBL: return KI_OEM_7; // "
        case SDLK_HASH: return KI_OEM_5; // #
        case SDLK_DOLLAR: return KI_OEM_4; // $
        case SDLK_AMPERSAND: return KI_OEM_7; // &
        case SDLK_QUOTE: return KI_OEM_7; // '
        case SDLK_LEFTPAREN: return KI_9; // (
        case SDLK_RIGHTPAREN: return KI_0; // )
        case SDLK_ASTERISK: return KI_8; // *
        case SDLK_PLUS: return KI_OEM_PLUS; // +
        case SDLK_COMMA: return KI_OEM_COMMA; // ,
        case SDLK_MINUS: return KI_OEM_MINUS; // -
        case SDLK_PERIOD: return KI_OEM_PERIOD; // .
        case SDLK_SLASH: return KI_OEM_2; // /
        case SDLK_0: return KI_0;
        case SDLK_1: return KI_1;
        case SDLK_2: return KI_2;
        case SDLK_3: return KI_3;
        case SDLK_4: return KI_4;
        case SDLK_5: return KI_5;
        case SDLK_6: return KI_6;
        case SDLK_7: return KI_7;
        case SDLK_8: return KI_8;
        case SDLK_9: return KI_9;
        case SDLK_COLON: return KI_OEM_1; // :
        case SDLK_SEMICOLON: return KI_OEM_1; // ;
        case SDLK_LESS: return KI_OEM_COMMA; // <
        case SDLK_EQUALS: return KI_OEM_PLUS; // =
        case SDLK_GREATER: return KI_OEM_PERIOD; // >
        case SDLK_QUESTION: return KI_OEM_2; // ?
        case SDLK_AT: return KI_2; // @
        case SDLK_LEFTBRACKET: return KI_OEM_4; // [
        case SDLK_BACKSLASH: return KI_OEM_5; // \
        case SDLK_RIGHTBRACKET: return KI_OEM_6; // ]
        case SDLK_CARET: return KI_6; // ^
        case SDLK_UNDERSCORE: return KI_OEM_MINUS; // _
        case SDLK_BACKQUOTE: return KI_OEM_3; // `
        case SDLK_a: return KI_A;
        case SDLK_b: return KI_B;
        case SDLK_c: return KI_C;
        case SDLK_d: return KI_D;
        case SDLK_e: return KI_E;
        case SDLK_f: return KI_F;
        case SDLK_g: return KI_G;
        case SDLK_h: return KI_H;
        case SDLK_i: return KI_I;
        case SDLK_j: return KI_J;
        case SDLK_k: return KI_K;
        case SDLK_l: return KI_L;
        case SDLK_m: return KI_M;
        case SDLK_n: return KI_N;
        case SDLK_o: return KI_O;
        case SDLK_p: return KI_P;
        case SDLK_q: return KI_Q;
        case SDLK_r: return KI_R;
        case SDLK_s: return KI_S;
        case SDLK_t: return KI_T;
        case SDLK_u: return KI_U;
        case SDLK_v: return KI_V;
        case SDLK_w: return KI_W;
        case SDLK_x: return KI_X;
        case SDLK_y: return KI_Y;
        case SDLK_z: return KI_Z;
        case SDLK_DELETE: return KI_DELETE;

        case SDLK_KP_0: return KI_NUMPAD0;
        case SDLK_KP_1: return KI_NUMPAD1;
        case SDLK_KP_2: return KI_NUMPAD2;
        case SDLK_KP_3: return KI_NUMPAD3;
        case SDLK_KP_4: return KI_NUMPAD4;
        case SDLK_KP_5: return KI_NUMPAD5;
        case SDLK_KP_6: return KI_NUMPAD6;
        case SDLK_KP_7: return KI_NUMPAD7;
        case SDLK_KP_8: return KI_NUMPAD8;
        case SDLK_KP_9: return KI_NUMPAD9;
        case SDLK_KP_PERIOD: return KI_DECIMAL;
        case SDLK_KP_DIVIDE: return KI_DIVIDE;
        case SDLK_KP_MULTIPLY: return KI_MULTIPLY;
        case SDLK_KP_MINUS: return KI_SUBTRACT;
        case SDLK_KP_PLUS: return KI_ADD;
        case SDLK_KP_ENTER: return KI_NUMPADENTER;
        case SDLK_KP_EQUALS: return KI_OEM_PLUS; // Not a direct match, but common for numpad equals

        case SDLK_UP: return KI_UP;
        case SDLK_DOWN: return KI_DOWN;
        case SDLK_RIGHT: return KI_RIGHT;
        case SDLK_LEFT: return KI_LEFT;
        case SDLK_INSERT: return KI_INSERT;
        case SDLK_HOME: return KI_HOME;
        case SDLK_END: return KI_END;
        case SDLK_PAGEUP: return KI_PRIOR;
        case SDLK_PAGEDOWN: return KI_NEXT;

        case SDLK_F1: return KI_F1;
        case SDLK_F2: return KI_F2;
        case SDLK_F3: return KI_F3;
        case SDLK_F4: return KI_F4;
        case SDLK_F5: return KI_F5;
        case SDLK_F6: return KI_F6;
        case SDLK_F7: return KI_F7;
        case SDLK_F8: return KI_F8;
        case SDLK_F9: return KI_F9;
        case SDLK_F10: return KI_F10;
        case SDLK_F11: return KI_F11;
        case SDLK_F12: return KI_F12;

        case SDLK_NUMLOCKCLEAR: return KI_NUMLOCK;
        case SDLK_CAPSLOCK: return KI_CAPITAL;
        case SDLK_SCROLLLOCK: return KI_SCROLL;
        case SDLK_RSHIFT: return KI_RSHIFT;
        case SDLK_LSHIFT: return KI_LSHIFT;
        case SDLK_RCTRL: return KI_RCONTROL;
        case SDLK_LCTRL: return KI_LCONTROL;
        // case SDLK_RALT: return KI_RALT;
        // case SDLK_LALT: return KI_LALT;
        case SDLK_MODE: return KI_MODECHANGE;
        case SDLK_PRINTSCREEN: return KI_SNAPSHOT;
        // case SDLK_MENU: return KI_MENU;
        // case SDLK_SYSREQ: return KI_SYSRQ;
        // case SDLK_BREAK: return KI_CANCEL; // Or KI_PAUSE, depending on exact meaning
        case SDLK_CLEARAGAIN: return KI_CLEAR; // Not a direct match, but closest

        default: return KI_UNKNOWN;
    }
}

// 辅助函数：获取 RmlUi 键盘修饰符
int GetRmlUiKeyModifiers() {
    int rml_modifiers = 0;
    const Uint8* state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT]) rml_modifiers |= Rml::Input::KM_SHIFT;
    if (state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL]) rml_modifiers |= Rml::Input::KM_CTRL;
    if (state[SDL_SCANCODE_LALT] || state[SDL_SCANCODE_RALT]) rml_modifiers |= Rml::Input::KM_ALT;
    // SDL does not provide a direct 'super' key (Windows key / Command key) modifier
    // You might need to map specific keys if you need this
    return rml_modifiers;
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
    
    // // ----------------------------------------------------
    // // 初始化 ImGui UiSystem
    // uiSystem_ = new UiSystem(window_, glContext_);

    // // 绑定 UiSystem 的回调函数到 Scene
    // uiSystem_->onCaptureButtonClicked = [this]()
    // {
    //     scene_->saveTextures();
    // };

    // uiSystem_->onToggleDebugButtonClicked = [this]()
    // {
    //     scene_->toggleDebugDraw();
    // };

    // // 绑定各种 UI 滑块的回调函数，以调整场景参数
    // uiSystem_->onSsrWeightBarChanged = [this](float weight)
    // {
    //     scene_->setSsrWeight(weight);
    // };
    // uiSystem_->ssrWeight = scene_->ssrWeight_;

    // uiSystem_->onIblWeightBarChanged = [this](float weight)
    // {
    //     scene_->setIblWeight(weight);
    // };
    // uiSystem_->iblWeight = scene_->iblWeight_;

    // uiSystem_->onLightWeightBarChanged = [this](float weight)
    // {
    //     scene_->setLightWeight(weight);
    // };
    // uiSystem_->lightWeight = scene_->lightWeight_;

    // uiSystem_->onOitWeightBarChanged = [this](float weight)
    // {
    //     scene_->setOitWeight(weight);
    // };
    // uiSystem_->oitWeight = scene_->oitWeight_;

    // uiSystem_->onGodRayWeightBarChanged = [this](float weight)
    // {
    //     scene_->setGodRayWeight(weight);
    // };
    // uiSystem_->godRayWeight = scene_->godRayWeight_;

    // uiSystem_->onFocusDistanceBarChanged = [this](float distance)
    // {
    //     scene_->setFocusDistance(distance);
    // };
    // uiSystem_->focusDistance = scene_->focusDistance_;

    // // !!! 关键改动 !!!
    // // onGetSelectedMesh 更改为 onGetSelectedObject，并返回 ISceneObject*
    // uiSystem_->onGetSelectedObject = [this]()
    // {
    //     return scene_->getSelectedObject();
    // };

    // ----------------------------------------------------
    // 初始化 RmlUi
    std::cout << "Initializing RmlUi..." << std::endl;
    rmlUiRenderer_ = new RmlUiOpenGLRenderer();
    if (!rmlUiRenderer_->Initialize()) {
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
    if (!Rml::Initialise()) { // <--- 修正为 Rml::Initialise
        std::cerr << "Failed to initialise RmlUi Core!" << std::endl;
        exit(-1);
    }

    // 加载字体
    LoadRmlUiFonts();

    // 创建 RmlUi 上下文
    rmlContext_ = Rml::CreateContext("main", Rml::Vector2i(w, h)); // <--- 修正为 Rml::CreateContext
    if (!rmlContext_) {
        std::cerr << "Failed to create RmlUi context!" << std::endl;
        exit(-1);
    }

    // 可选：初始化 RmlUi 调试器
    // #ifdef RMLUI_DEBUGGER_ENABLED // 如果你在 CMake 中定义了这个宏
    // Rml::Debugger::Initialise(rmlContext_); // <--- 修正为 Rml::Debugger
    // Rml::Debugger::SetVisible(true); // 默认显示调试器
    // #endif

    // 加载并显示 RML 文档
    rmlDocument_ = rmlContext_->LoadDocument("sample.rml");
    if (rmlDocument_) {
        rmlDocument_->Show();
        std::cout << "RmlUi sample.rml loaded and shown." << std::endl;

        // 绑定按钮事件 (例如关闭窗口)
        Rml::Element* closeButton = rmlDocument_->GetElementById("close_button"); // <--- 修正为 Rml::Element
        if (closeButton) {
            // closeButton->AddEventListener(Rml::EventId::Click, [this](Rml::Event& event) { // <--- 修正为 Rml::EventId, Rml::Event
            //     this->running_ = false; // 点击按钮关闭窗口
            //     std::cout << "Close button clicked! Exiting application." << std::endl;
            // });
        }
    } else {
        std::cerr << "Failed to load RmlUi document: sample.rml" << std::endl;
    }


    // ----------------------------------------------------
    // 其他初始化
    lastFPSTime_ = std::chrono::high_resolution_clock::now();
    lastFrameTime_ = std::chrono::high_resolution_clock::now();

    // 假设 UiSceneData::create 方法存在，用于从 Scene 初始化 UI 数据
    // 确保 UiSceneData 也能处理 ISceneObject
    // uiSystem_->uiSceneData = UiSceneData::create(scene_);

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
    // ----------------------------------------------------
    // 清理 RmlUi 资源
    std::cout << "Shutting down RmlUi..." << std::endl;
    if (rmlDocument_) {
        rmlDocument_->Close();
        rmlDocument_ = nullptr;
    }
    if (rmlContext_) {
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
    // delete uiSystem_;
    // uiSystem_ = nullptr;

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
        snprintf(title, sizeof(title), "OpenGL + SDL2 + ImGui + RmlUi - FPS: %d", fps_); // <--- 更新标题
        SDL_SetWindowTitle(window_, title);

        // 更新 RmlUi 中的 FPS 显示
        if (rmlDocument_) {
            Rml::Element* fpsElement = rmlDocument_->GetElementById("fps_display"); // <--- 修正为 Rml::Element
            if (fpsElement) {
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

    // --- 更新命令的 deltaTime (针对需要它的命令) ---
    if (cmd_moveForward_) cmd_moveForward_->setDeltaTime(deltaTime_);
    if (cmd_moveBackward_) cmd_moveBackward_->setDeltaTime(deltaTime_);
    if (cmd_moveLeft_) cmd_moveLeft_->setDeltaTime(deltaTime_);
    if (cmd_moveRight_) cmd_moveRight_->setDeltaTime(deltaTime_);
    if (cmd_moveUp_) cmd_moveUp_->setDeltaTime(deltaTime_);
    if (cmd_moveDown_) cmd_moveDown_->setDeltaTime(deltaTime_);
    if (cmd_rotateLeft_) cmd_rotateLeft_->setDeltaTime(deltaTime_);
    if (cmd_rotateRight_) cmd_rotateRight_->setDeltaTime(deltaTime_);

    // --- 输入处理 ---
    InputManager::GetInstance().Update();

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        // 先让 ImGui 处理事件
        // uiSystem_->ProcessEvent(&event);
        
        // 再让 RmlUi 处理事件
        // 关键：检查 ImGui 是否捕获了事件，如果捕获了，RmlUi 不应处理
        bool ImGuiWantsMouse = ImGui::GetIO().WantCaptureMouse;
        bool ImGuiWantsKeyboard = ImGui::GetIO().WantCaptureKeyboard;

        if (event.type == SDL_QUIT)
        {
            running_ = false;
        }
        else if (event.type == SDL_WINDOWEVENT)
        {
            if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
                event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                int w, h;
                SDL_GL_GetDrawableSize(window_, &w, &h);
                glViewport(0, 0, w, h);
                scene_->resize(w, h);
                scene_->getCamera()->setAspectRatio(static_cast<float>(w) / h);
                // 通知 RmlUi 窗口大小改变
                if (rmlContext_) {
                    rmlContext_->SetDimensions(Rml::Vector2i(w, h)); // <--- 修正为 Rml::Vector2i
                    rmlUiRenderer_->SetViewport(w, h); // 更新 RmlUi 渲染器的投影矩阵
                }
            }
            // RmlUi 也需要窗口事件，例如焦点改变。
            // RmlUi 6.x 的 SystemInterface 不直接提供 ProcessWindowEvent，
            // 但 ProcessKey 可以用于模拟焦点事件。
            // if (rmlContext_ && !ImGuiWantsKeyboard) { 
            //     switch (event.window.event) {
            //         case SDL_WINDOWEVENT_FOCUS_GAINED: 
            //             rmlContext_->ProcessKey(Rml::Input::KI_HOME, 0, true); // 模拟焦点获得
            //             break;
            //         case SDL_WINDOWEVENT_FOCUS_LOST: 
            //             rmlContext_->ProcessKey(Rml::Input::KI_END, 0, true); // 模拟焦点丢失
            //             break;
            //         // 可以根据需要添加更多
            //     }
            // }
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
        {
            if (rmlContext_ && !ImGuiWantsMouse) {
                rmlContext_->ProcessMouseButtonDown(event.button.button - 1, event.type == SDL_MOUSEBUTTONDOWN);
            }
        }
        else if (event.type == SDL_MOUSEMOTION)
        {
            if (rmlContext_ && !ImGuiWantsMouse) {
                rmlContext_->ProcessMouseMove(event.motion.x, event.motion.y, GetRmlUiKeyModifiers()); // <--- 添加修饰符
            }
        }
        else if (event.type == SDL_MOUSEWHEEL)
        {
            if (rmlContext_ && !ImGuiWantsMouse) {
                rmlContext_->ProcessMouseWheel(event.wheel.y, GetRmlUiKeyModifiers()); // <--- 添加修饰符
            }
        }
        else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
        {
            if (rmlContext_ && !ImGuiWantsKeyboard) {
                Rml::Input::KeyIdentifier key_id = SDLKeyToRmlKey(event.key.keysym.sym); // 使用辅助函数映射
                int rml_modifiers = GetRmlUiKeyModifiers(); // 使用辅助函数获取修饰符

                // rmlContext_->ProcessKeyDown(key_id, rml_modifiers, event.type == SDL_KEYDOWN);
            }
        }
        else if (event.type == SDL_TEXTINPUT)
        {
            if (rmlContext_ && !ImGuiWantsKeyboard) {
                rmlContext_->ProcessTextInput(event.text.text);
            }
        }
        
        // 最后让 InputManager 处理事件 (用于游戏逻辑)
        InputManager::GetInstance().ProcessEvent(event);
    }

    // --- RmlUi 更新 ---
    if (rmlContext_) {
        rmlContext_->Update();
    }

    // --- 命令调度 (Invoke Commands) ---
    // 只有当 RmlUi 和 ImGui 都没有捕获键盘/鼠标时，才执行游戏输入
    bool uiCapturesMouse = (ImGui::GetIO().WantCaptureMouse || (rmlContext_ && rmlContext_->IsMouseInteracting()));
    bool uiCapturesKeyboard = (ImGui::GetIO().WantCaptureKeyboard || (rmlContext_ && rmlContext_->GetFocusElement() != nullptr));

    if (InputManager::GetInstance().IsKeyPressed(SDL_SCANCODE_ESCAPE))
    {
        running_ = false;
    }

    if (!uiCapturesKeyboard) { // 只有当 UI 没有捕获键盘时，才处理游戏键盘输入
        if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_W)) cmd_moveForward_->Execute();
        if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_S)) cmd_moveBackward_->Execute();
        if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_A)) cmd_moveLeft_->Execute();
        if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_D)) cmd_moveRight_->Execute();
        if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_Q)) cmd_moveUp_->Execute();
        if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_E)) cmd_moveDown_->Execute();
        if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_Z)) cmd_rotateLeft_->Execute();
        if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_C)) cmd_rotateRight_->Execute();

        if (InputManager::GetInstance().IsKeyPressed(SDL_SCANCODE_F1))
        {
            cmd_toggleDebug_->Execute();
        }
    }

    if (!uiCapturesMouse) { // 只有当 UI 没有捕获鼠标时，才处理游戏鼠标输入
        cmd_mouseLook_->Execute();
        cmd_mouseScroll_->Execute();

        if (InputManager::GetInstance().IsMouseButtonPressed(SDL_BUTTON_LEFT))
        {
            cmd_pickObject_->setMousePosition(InputManager::GetInstance().GetMouseX(), InputManager::GetInstance().GetMouseY());
            cmd_pickObject_->Execute();

            pickedObject_ = cmd_pickObject_->getPickedObject(); 

            if (pickedObject_)
            {
                scene_->setSelectedObject(pickedObject_); 
                std::cout << "Picked object: " << pickedObject_->getName() << std::endl;
            }
            else
            {
                scene_->setSelectedObject(nullptr); 
                std::cout << "No object picked." << std::endl;
            }
        }
    } else {
        // 当 UI 捕获鼠标时，如果需要，仍然可以打印上一个选中对象的信息
        // if (pickedObject_) {
        //     std::cout << "UI capturing mouse. Last picked object: " << pickedObject_->getName() << std::endl;
        // } else {
        //     std::cout << "UI capturing mouse. No object picked." << std::endl;
        // }
    }


    // --- 更新 FPS ---
    updateFPS();
}

// 渲染场景和 UI
void Window::render()
{
    // // 1. 开始 ImGui 新帧的绘制
    // uiSystem_->NewFrame();

    // // 2. 绘制自定义 ImGui UI 界面
    // uiSystem_->DrawUI(fps_);

    // // 3. 清除颜色缓冲区和深度缓冲区
    // glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 4. 执行场景的渲染逻辑
    scene_->run(deltaTime_);

    // 5. 渲染 RmlUi 界面
    if (rmlContext_) {
        // RmlUi 渲染器会自己设置 OpenGL 状态，并在渲染后恢复
        rmlContext_->Render();
    }

    // 6. 渲染 ImGui 界面到 OpenGL
    // uiSystem_->Render();

    // 7. 交换窗口缓冲
    SDL_GL_SwapWindow(window_);
}

// 检查窗口是否仍在运行
bool Window::isRunning()
{
    return running_;
}
