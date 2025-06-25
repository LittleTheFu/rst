#ifndef CAMERA_COMMANDS_H
#define CAMERA_COMMANDS_H

#include "Command.h" // 包含 Command 基类
#include "Camera.h"  // 包含 Camera 类
#include <inputManager.h>
#include <iostream>

// 前向声明，防止循环引用
class Camera;

// ----------------------------------------------------
// 抽象摄像机命令基类 (可选，但推荐)
// 用于封装对 Camera 的操作，如果未来有更多 Camera 相关的命令，可继承它
class CameraCommand : public Command {
protected:
    Camera* camera_;
    float deltaTime_; // 命令可能需要 deltaTime 来进行时间相关的操作

public:
    CameraCommand(Camera* camera, float dt) : camera_(camera), deltaTime_(dt) {}
    // Execute 仍由派生类实现
};

// ----------------------------------------------------
// 具体摄像机移动命令

class MoveCameraForwardCommand : public CameraCommand {
public:
    MoveCameraForwardCommand(Camera* camera, float dt) : CameraCommand(camera, dt) {}
    void Execute() override {
        if (camera_) camera_->ProcessKeyboard(FORWARD, deltaTime_);
    }
};

class MoveCameraBackwardCommand : public CameraCommand {
public:
    MoveCameraBackwardCommand(Camera* camera, float dt) : CameraCommand(camera, dt) {}
    void Execute() override {
        if (camera_) camera_->ProcessKeyboard(BACKWARD, deltaTime_);
    }
};

class MoveCameraLeftCommand : public CameraCommand {
public:
    MoveCameraLeftCommand(Camera* camera, float dt) : CameraCommand(camera, dt) {}
    void Execute() override {
        if (camera_) camera_->ProcessKeyboard(LEFT, deltaTime_);
    }
};

class MoveCameraRightCommand : public CameraCommand {
public:
    MoveCameraRightCommand(Camera* camera, float dt) : CameraCommand(camera, dt) {}
    void Execute() override {
        if (camera_) camera_->ProcessKeyboard(RIGHT, deltaTime_);
    }
};

class MoveCameraUpCommand : public CameraCommand {
public:
    MoveCameraUpCommand(Camera* camera, float dt) : CameraCommand(camera, dt) {}
    void Execute() override {
        if (camera_) camera_->ProcessKeyboard(UP, deltaTime_);
    }
};

class MoveCameraDownCommand : public CameraCommand {
public:
    MoveCameraDownCommand(Camera* camera, float dt) : CameraCommand(camera, dt) {}
    void Execute() override {
        if (camera_) camera_->ProcessKeyboard(DOWN, deltaTime_);
    }
};

// ----------------------------------------------------
// 鼠标视角命令
// 注意：鼠标命令的 Execute() 内部可能需要从 InputManager 获取 mouseDeltaX/Y
// 我们可以通过构造函数传入 InputManager 引用，或者让命令本身去获取单例。
// 为了简洁，我们让它直接获取单例。

class ProcessMouseMovementCommand : public Command {
private:
    Camera* camera_;

public:
    ProcessMouseMovementCommand(Camera* camera) : camera_(camera) {}
    void Execute() override {
        if (camera_) {
            InputManager& input = InputManager::GetInstance();
            camera_->ProcessMouseMovement(input.GetMouseDeltaX(), input.GetMouseDeltaY());
        }
    }
};

// ----------------------------------------------------
// 鼠标滚轮命令

class ProcessMouseScrollCommand : public Command {
private:
    Camera* camera_;

public:
    ProcessMouseScrollCommand(Camera* camera) : camera_(camera) {}
    void Execute() override {
        if (camera_) {
            InputManager& input = InputManager::GetInstance();
            if (input.GetMouseScrollY() != 0) { // 只有滚轮滚动时才调用
                camera_->ProcessMouseScroll(input.GetMouseScrollY());
            }
        }
    }
};

// ----------------------------------------------------
// 其他通用命令示例

class ToggleDebugModeCommand : public Command {
    // Scene* scene_; // 假设有一个场景指针
public:
    // ToggleDebugModeCommand(Scene* scene) : scene_(scene) {}
    void Execute() override {
        std::cout << "Debug Mode Toggled!" << std::endl;
        // if (scene_) scene_->toggleDebugMode(); // 调用场景的调试模式切换方法
    }
};

#endif // CAMERA_COMMANDS_H