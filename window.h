#ifndef WINDOW_H
#define WINDOW_H

#include <SDL.h>
#include <glad/glad.h>
#include <chrono>   // 用于时间计算
#include <iostream> // 用于输出调试信息
#include <glm/glm.hpp> // 用于 glm::mat4 (如果 scene_ 或其他组件需要)

// 包含项目中的其他核心组件
#include "scene.h"        // 你的场景类
#include "uiSystem.h"   // 你的 UI 系统
#include "camera.h"       // Camera 类
#include "InputManager.h" // InputManager

// 包含命令模式相关的头文件
#include "Command.h"      // 抽象命令基类
#include "moveCameraForwardCommand.h"
#include "moveCameraBackwardCommand.h"
#include "moveCameraLeftCommand.h"
#include "moveCameraRightCommand.h"
#include "moveCameraUpCommand.h"
#include "moveCameraDownCommand.h"
#include "processMouseMovementCommand.h"
#include "processMouseScrollCommand.h"
#include "toggleDebugModeCommand.h"
#include "pickObjectCommand.h"

class Window {
public:
    // 构造函数：初始化 SDL, OpenGL 上下文, GLAD, UI 系统, 场景和命令
    Window(const char* title, int width, int height);
    // 析构函数：清理所有资源 (SDL, OpenGL 上下文, UI 系统, 命令对象)
    ~Window();

    // 更新帧率计数器和窗口标题
    void updateFPS();
    
    // 处理所有输入事件、更新游戏逻辑和摄像机状态。
    // 这个方法应该在主循环中每帧调用一次。
    void update();

    // 渲染场景和 UI。
    // 在这里，摄像机的视图和投影矩阵会被传递给场景进行渲染。
    void render();

    // 检查窗口是否仍在运行。
    // 它返回一个内部标志，该标志由 update() 中的事件处理更新。
    bool isRunning();

private:
    SDL_Window* window_ = nullptr;     // SDL 窗口指针
    SDL_GLContext glContext_;          // OpenGL 上下文

    // FPS 相关成员变量
    int fps_ = 0;
    int frameCount_ = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastFPSTime_;

    // DeltaTime 相关成员变量
    std::chrono::time_point<std::chrono::high_resolution_clock> lastFrameTime_;
    float deltaTime_ = 0.0f; // 存储当前帧的 deltaTime (秒)

    Scene scene_;                 // 你的游戏场景实例
    UiSystem* uiSystem_ = nullptr; // UI 系统指针

    Camera camera_; // 摄像机实例

    bool running_ = true; // 控制主循环的内部标志，true 表示程序正在运行

    // --- 命令对象指针 ---
    // 这些是具体命令类的实例，它们将在构造函数中被创建，在析构函数中被删除。
    // 每个指针指向一个特定的命令实例，负责执行一个特定的动作。
    MoveCameraForwardCommand* cmd_moveForward_ = nullptr;
    MoveCameraBackwardCommand* cmd_moveBackward_ = nullptr;
    MoveCameraLeftCommand* cmd_moveLeft_ = nullptr;
    MoveCameraRightCommand* cmd_moveRight_ = nullptr;
    MoveCameraUpCommand* cmd_moveUp_ = nullptr;
    MoveCameraDownCommand* cmd_moveDown_ = nullptr;
    ProcessMouseMovementCommand* cmd_mouseLook_ = nullptr;
    ProcessMouseScrollCommand* cmd_mouseScroll_ = nullptr;
    ToggleDebugModeCommand* cmd_toggleDebug_ = nullptr; // 示例：调试模式切换命令
    PickObjectCommand* cmd_pickObject_ = nullptr;

    Mesh *pickedMesh_ = nullptr;
};

#endif // WINDOW_H