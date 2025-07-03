// Window.h (修改后)
#ifndef WINDOW_H
#define WINDOW_H

#include <SDL.h>
#include <glad/glad.h>
#include <chrono> // 用于时间计算
#include <iostream> // 用于输出调试信息

// 包含项目中的其他核心组件
#include "scene.h"// 你的场景类
#include "uiSystem.h" // 你的 UI 系统 (ImGui)
#include "camera.h" // Camera 类
#include "InputManager.h" // InputManager
#include "sceneObject.h" // 引入 ISceneObject 接口

// 包含命令模式相关的头文件
#include "Command.h"// 抽象命令基类 (需要 CameraCommand 继承 Command 并有 setDeltaTime 方法)
#include "moveCameraForwardCommand.h"
#include "moveCameraBackwardCommand.h"
#include "moveCameraLeftCommand.h"
#include "moveCameraRightCommand.h"
#include "moveCameraUpCommand.h"
#include "moveCameraDownCommand.h"
#include "processMouseMovementCommand.h"
#include "processMouseScrollCommand.h"
#include "toggleDebugModeCommand.h"
#include "pickObjectCommand.h" // 此命令需要改造以返回 ISceneObject*
#include "rotateCameraLeftCommand.h"
#include "rotateCameraRightCommand.h"

// RmlUi 相关头文件 (注意命名空间，根据你的 RmlUi 版本调整)
#include <RmlUi/Core.h> // 包含 Rml::Initialise, Rml::Shutdown, Rml::CreateContext 等
#include <RmlUi/Core/Context.h> // Rml::Context
#include <RmlUi/Core/ElementDocument.h> // Rml::ElementDocument
#include <RmlUi/Debugger.h> // 可选：用于 RmlUi 调试器 (Rml::Debugger)

// 你自定义的接口实现
#include "RmlUiOpenGLRenderer.h"
#include "RmlUiSystemInterface.h"
#include "RmlUiFileInterface.h"

#include "GameInputProcessor.h"
#include "RmlUiInputProcessor.h"
#include "ImGuiInputProcessor.h"


class Window {
public:
    // 构造函数：初始化 SDL, OpenGL 上下文, GLAD, UI 系统, 场景和命令
    Window(const char* title, int width, int height);
    // 析构函数：清理所有资源 (SDL, OpenGL 上下文, UI 系统, 命令对象)
    ~Window();

    // 更新帧率计数器和窗口标题
    void updateFPS();
    
    // 处理所有输入事件、更新游戏逻辑和摄像机状态。
    void update();

    // 渲染场景和 UI。
    void render();

    // 检查窗口是否仍在运行。
    bool isRunning();

private:
    SDL_Window* window_ = nullptr; 
    SDL_GLContext glContext_; 

    // FPS 相关成员变量
    int fps_ = 0;
    int frameCount_ = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastFPSTime_;

    // DeltaTime 相关成员变量
    std::chrono::time_point<std::chrono::high_resolution_clock> lastFrameTime_;
    float deltaTime_ = 0.0f; // 存储当前帧的 deltaTime (秒)

    std::shared_ptr<Scene> scene_; 
    UiSystem* uiSystem_ = nullptr; // 你的 ImGui UI 系统

    bool running_ = true; 

    // --- 命令对象指针 ---
    MoveCameraForwardCommand* cmd_moveForward_ = nullptr;
    MoveCameraBackwardCommand* cmd_moveBackward_ = nullptr;
    MoveCameraLeftCommand* cmd_moveLeft_ = nullptr;
    MoveCameraRightCommand* cmd_moveRight_ = nullptr;
    MoveCameraUpCommand* cmd_moveUp_ = nullptr;
    MoveCameraDownCommand* cmd_moveDown_ = nullptr;
    RotateCameraLeftCommand* cmd_rotateLeft_ = nullptr;
    RotateCameraRightCommand* cmd_rotateRight_ = nullptr;
    ProcessMouseMovementCommand* cmd_mouseLook_ = nullptr;
    ProcessMouseScrollCommand* cmd_mouseScroll_ = nullptr;
    ToggleDebugModeCommand* cmd_toggleDebug_ = nullptr; 
    PickObjectCommand* cmd_pickObject_ = nullptr;

    // 修改 pickedMesh_ 为 pickedObject_
    ISceneObject* pickedObject_ = nullptr; 

    // --- RmlUi 相关成员变量 ---
    RmlUiOpenGLRenderer* rmlUiRenderer_ = nullptr;
    RmlUiSystemInterface* rmlUiSystemInterface_ = nullptr;
    RmlUiFileInterface* rmlUiFileInterface_ = nullptr;
    Rml::Context* rmlContext_ = nullptr; // <--- 修正为 Rml::Context
    Rml::ElementDocument* rmlDocument_ = nullptr; // <--- 修正为 Rml::ElementDocument

    // --- NEW INPUT SYSTEM COMPONENTS ---
    std::unique_ptr<CommandQueue> commandQueue_; // 命令队列
    std::vector<std::unique_ptr<AbstractInputProcessor>> inputProcessors_; // 输入处理器链

    ImGuiInputProcessor* imGuiProcessor_; // Raw pointer to avoid unique_ptr issues with vector ownership
    RmlUiInputProcessor* rmlUiProcessor_;
    GameInputProcessor* gameInputProcessor_;
};

#endif // WINDOW_H
