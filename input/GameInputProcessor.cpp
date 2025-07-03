#include "GameInputProcessor.h"
#include "InputManager.h" // 确保包含 InputManager
#include "camera.h"       // 确保包含 Camera
#include "scene.h"        // 确保包含 Scene
// 包含所有具体命令的头文件
#include "MoveCameraForwardCommand.h"
#include "MoveCameraBackwardCommand.h"
#include "MoveCameraLeftCommand.h"
#include "MoveCameraRightCommand.h"
#include "MoveCameraUpCommand.h"
#include "MoveCameraDownCommand.h"
#include "RotateCameraLeftCommand.h"
#include "RotateCameraRightCommand.h"
#include "ProcessMouseMovementCommand.h"
#include "ProcessMouseScrollCommand.h"
#include "ToggleDebugModeCommand.h"
#include "PickObjectCommand.h"
#include <iostream> // 用于调试输出

GameInputProcessor::GameInputProcessor(std::shared_ptr<Camera> camera, std::shared_ptr<Scene> scene)
    : camera_(camera), scene_(scene) {
    // 初始化命令对象。注意这里使用 unique_ptr，
    // 它们的所有权将由 GameInputProcessor 管理。
    // 确保你的 CameraCommand 及其派生类构造函数接受 shared_ptr<Camera>
    // 以及 Scene 和 ObjectPicker 的相应指针。
    cmd_moveForward_ = std::make_unique<MoveCameraForwardCommand>(camera_);
    cmd_moveBackward_ = std::make_unique<MoveCameraBackwardCommand>(camera_);
    cmd_moveLeft_ = std::make_unique<MoveCameraLeftCommand>(camera_);
    cmd_moveRight_ = std::make_unique<MoveCameraRightCommand>(camera_);
    cmd_moveUp_ = std::make_unique<MoveCameraUpCommand>(camera_);
    cmd_moveDown_ = std::make_unique<MoveCameraDownCommand>(camera_);
    cmd_rotateLeft_ = std::make_unique<RotateCameraLeftCommand>(camera_);
    cmd_rotateRight_ = std::make_unique<RotateCameraRightCommand>(camera_);

    cmd_mouseLook_ = std::make_unique<ProcessMouseMovementCommand>(camera_);
    cmd_mouseScroll_ = std::make_unique<ProcessMouseScrollCommand>(camera_);

    cmd_toggleDebug_ = std::make_unique<ToggleDebugModeCommand>(scene_);

    // 确保 scene_->objectPicker_ 是可访问的 unique_ptr 或 shared_ptr
    cmd_pickObject_ = std::make_unique<PickObjectCommand>(scene_->objectPicker_.get());
}

bool GameInputProcessor::ProcessEvent(const SDL_Event& event, CommandQueue& commandQueue) {
    // GameInputProcessor 的 ProcessEvent 职责单一：将原始事件传递给 InputManager
    // InputManager 负责缓存所有原始输入状态。
    // 注意：这里不判断事件是否被UI捕获，因为 InputManager 总是需要所有原始事件。
    InputManager::GetInstance().ProcessEvent(event);

    // GameInputProcessor 不“消费”事件，因为它只是将原始事件传给 InputManager 缓存。
    // 实际的命令生成和优先级判断在 GenerateCommands 阶段。
    return false;
}

void GameInputProcessor::GenerateCommands(CommandQueue& commandQueue, float deltaTime, bool uiCapturesKeyboard, bool uiCapturesMouse) {
    // 更新相机命令的 deltaTime
    cmd_moveForward_->setDeltaTime(deltaTime);
    cmd_moveBackward_->setDeltaTime(deltaTime);
    cmd_moveLeft_->setDeltaTime(deltaTime);
    cmd_moveRight_->setDeltaTime(deltaTime);
    cmd_moveUp_->setDeltaTime(deltaTime);
    cmd_moveDown_->setDeltaTime(deltaTime);
    cmd_rotateLeft_->setDeltaTime(deltaTime);
    cmd_rotateRight_->setDeltaTime(deltaTime);

    // 首先处理退出请求，这通常是最高优先级，无论UI是否捕获
    if (InputManager::GetInstance().IsQuitRequested()) {
        // 通常会有一个 QuitApplicationCommand，这里假设直接设置运行状态
        // commandQueue.AddCommand(std::make_unique<QuitApplicationCommand>());
        // 注意：Window::isRunning() 的控制逻辑将移到 Window 自身
    }

    // 只有当 UI 没有捕获键盘输入时，才处理游戏键盘命令
    if (!uiCapturesKeyboard) {
        if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_W))
            commandQueue.AddCommand(std::move(cmd_moveForward_)); // 转移所有权
        if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_S))
            commandQueue.AddCommand(std::move(cmd_moveBackward_));
        if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_A))
            commandQueue.AddCommand(std::move(cmd_moveLeft_));
        if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_D))
            commandQueue.AddCommand(std::move(cmd_moveRight_));
        if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_Q))
            commandQueue.AddCommand(std::move(cmd_moveUp_));
        if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_E))
            commandQueue.AddCommand(std::move(cmd_moveDown_));
        if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_Z))
            commandQueue.AddCommand(std::move(cmd_rotateLeft_));
        if (InputManager::GetInstance().IsKeyDown(SDL_SCANCODE_C))
            commandQueue.AddCommand(std::move(cmd_rotateRight_));

        if (InputManager::GetInstance().IsKeyPressed(SDL_SCANCODE_F1)) {
            commandQueue.AddCommand(std::move(cmd_toggleDebug_));
        }

        // 重新创建已转移的 unique_ptr，因为命令是“一次性”的
        // 这是一个关键的设计选择：
        // 1. 命令对象是临时的，每帧创建并执行后销毁。
        //    优点：简单，避免状态管理。缺点：频繁创建小对象开销。
        // 2. 命令对象是长寿的，每次执行后重置状态。
        //    优点：减少内存分配。缺点：需要命令实现 Reset() 方法。
        // 这里为了演示方便，采用第一种（每帧创建）。
        // 如果命令对象比较复杂，或者希望减少分配，可以采用第二种，
        // 即命令执行后不会被unique_ptr删除，而是被pop出队列后，可以重新push
        // 这时cmd_moveForward_等将作为裸指针，或者 CommandQueue 返回拥有权，更复杂。
        // 针对你的现有设计 (cmd_moveForward_是成员变量)，我们选择**每次AddCommand后重新创建**。
        cmd_moveForward_ = std::make_unique<MoveCameraForwardCommand>(camera_);
        cmd_moveBackward_ = std::make_unique<MoveCameraBackwardCommand>(camera_);
        cmd_moveLeft_ = std::make_unique<MoveCameraLeftCommand>(camera_);
        cmd_moveRight_ = std::make_unique<MoveCameraRightCommand>(camera_);
        cmd_moveUp_ = std::make_unique<MoveCameraUpCommand>(camera_);
        cmd_moveDown_ = std::make_unique<MoveCameraDownCommand>(camera_);
        cmd_rotateLeft_ = std::make_unique<RotateCameraLeftCommand>(camera_);
        cmd_rotateRight_ = std::make_unique<RotateCameraRightCommand>(camera_);
        cmd_toggleDebug_ = std::make_unique<ToggleDebugModeCommand>(scene_);

    }

    // 只有当 UI 没有捕获鼠标输入时，才处理游戏鼠标命令
    if (!uiCapturesMouse) {
        // 鼠标移动命令
        float mouseDeltaX = InputManager::GetInstance().GetMouseDeltaX();
        float mouseDeltaY = InputManager::GetInstance().GetMouseDeltaY();
        if (mouseDeltaX != 0.0f || mouseDeltaY != 0.0f) {
            // cmd_mouseLook_->setMouseDelta(mouseDeltaX, mouseDeltaY);
            commandQueue.AddCommand(std::move(cmd_mouseLook_));
            cmd_mouseLook_ = std::make_unique<ProcessMouseMovementCommand>(camera_); // 重新创建
        }

        // 鼠标滚轮命令
        float mouseScrollY = InputManager::GetInstance().GetMouseScrollY();
        if (mouseScrollY != 0.0f) {
            // cmd_mouseScroll_->setScrollDelta(mouseScrollY);
            commandQueue.AddCommand(std::move(cmd_mouseScroll_));
            cmd_mouseScroll_ = std::make_unique<ProcessMouseScrollCommand>(camera_); // 重新创建
        }

        // 鼠标左键点击拾取对象
        if (InputManager::GetInstance().IsMouseButtonPressed(SDL_BUTTON_LEFT)) {
            int mouseX = InputManager::GetInstance().GetMouseX();
            int mouseY = InputManager::GetInstance().GetMouseY();
            // PickObjectCommand 需要知道当前的鼠标位置
            cmd_pickObject_->setMousePosition(mouseX, mouseY);
            commandQueue.AddCommand(std::move(cmd_pickObject_));
            cmd_pickObject_ = std::make_unique<PickObjectCommand>(scene_->objectPicker_.get()); // 重新创建
        }
    }
}