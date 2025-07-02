#ifndef GAME_INPUT_PROCESSOR_H
#define GAME_INPUT_PROCESSOR_H

#include "AbstractInputProcessor.h"
#include "InputManager.h" // 需要 InputManager 来查询输入状态
#include "ICommand.h"     // 需要 ICommand
#include "camera.h"       // 需要 Camera 来创建相机相关的命令
#include "scene.h"        // 需要 Scene 来创建场景相关的命令
#include <memory>         // 用于智能指针

// 前向声明具体命令，以避免循环依赖和包含不必要的头文件
class MoveCameraForwardCommand;
class MoveCameraBackwardCommand;
class MoveCameraLeftCommand;
class MoveCameraRightCommand;
class MoveCameraUpCommand;
class MoveCameraDownCommand;
class RotateCameraLeftCommand;
class RotateCameraRightCommand;
class ProcessMouseMovementCommand;
class ProcessMouseScrollCommand;
class ToggleDebugModeCommand;
class PickObjectCommand;

// 游戏输入处理器
class GameInputProcessor : public AbstractInputProcessor {
public:
    // 构造函数需要各种依赖，例如 Camera、Scene、ObjectPicker 等，
    // 以便在 GenerateCommands 阶段创建相应的命令
    GameInputProcessor(std::shared_ptr<Camera> camera, std::shared_ptr<Scene> scene);

    // 处理 SDL 事件。
    // GameInputProcessor 的 ProcessEvent 只负责将原始事件传递给 InputManager 缓存，
    // 不在此阶段生成命令，也不“消费”事件。
    bool ProcessEvent(const SDL_Event& event, CommandQueue& commandQueue) override;

    // GameInputProcessor 不需要特殊的 BeginFrame 逻辑，因为它依赖于 InputManager 的 Update()
    void BeginFrame() override {
        // 通常不需要在这里做任何事情，InputManager::Update() 已在 Window::update() 头部调用
    }

    // 游戏输入处理器本身不捕获输入，它仅在UI不捕获时才生成命令。
    // 因此，这些方法始终返回 false。
    bool WantsToCaptureMouse() const override { return false; }
    bool WantsToCaptureKeyboard() const override { return false; }

    // 在 SDL 事件循环结束后调用此方法，根据 UI 捕获状态和 InputManager 的状态生成游戏命令。
    // 这是游戏逻辑判断和命令生成的核心。
    void GenerateCommands(CommandQueue& commandQueue, float deltaTime, bool uiCapturesKeyboard, bool uiCapturesMouse);

private:
    std::shared_ptr<Camera> camera_;
    std::shared_ptr<Scene> scene_; // 需要 Scene 来获取 ObjectPicker 或其他 Scene 相关的状态/操作

    // 你当前在 Window 中创建的命令对象可以移动到这里，
    // 或者更灵活地，直接在这里创建 unique_ptr<Command> 并添加到队列。
    // 为了简化，我们假设一些常用的命令在构造函数中被初始化，
    // 但更高级的命令可能在 GenerateCommands 中动态创建。
    std::unique_ptr<MoveCameraForwardCommand> cmd_moveForward_;
    std::unique_ptr<MoveCameraBackwardCommand> cmd_moveBackward_;
    std::unique_ptr<MoveCameraLeftCommand> cmd_moveLeft_;
    std::unique_ptr<MoveCameraRightCommand> cmd_moveRight_;
    std::unique_ptr<MoveCameraUpCommand> cmd_moveUp_;
    std::unique_ptr<MoveCameraDownCommand> cmd_moveDown_;
    std::unique_ptr<RotateCameraLeftCommand> cmd_rotateLeft_;
    std::unique_ptr<RotateCameraRightCommand> cmd_rotateRight_;
    std::unique_ptr<ProcessMouseMovementCommand> cmd_mouseLook_;
    std::unique_ptr<ProcessMouseScrollCommand> cmd_mouseScroll_;
    std::unique_ptr<ToggleDebugModeCommand> cmd_toggleDebug_;
    std::unique_ptr<PickObjectCommand> cmd_pickObject_;
};

#endif // GAME_INPUT_PROCESSOR_H