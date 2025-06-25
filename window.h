#ifndef WINDOW_H
#define WINDOW_H

#include <SDL.h>
#include <glad/glad.h>
#include <chrono>
#include <iostream>
#include <glm/glm.hpp> // For glm::mat4

#include "scene.h"      // 你的场景类
#include "UiSystem.h"   // 你的 UI 系统
#include "InputManager.h" // 新增：包含 InputManager

class Window {
public:
    Window(const char* title, int width, int height);
    ~Window();

    // 更新 FPS 计数器和窗口标题
    void updateFPS();
    
    // 处理所有输入、更新游戏逻辑，包括摄像机移动。
    // 这个方法应该在主循环中每帧调用一次。
    void update();

    // 渲染场景和 UI。
    void render();

    // 检查窗口是否仍在运行。现在只返回一个内部标志。
    bool isRunning();

private:
    SDL_Window* window_ = nullptr;
    SDL_GLContext glContext_;

    // FPS 相关
    int fps_ = 0;
    int frameCount_ = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastFPSTime_;

    // DeltaTime 相关
    std::chrono::time_point<std::chrono::high_resolution_clock> lastFrameTime_;
    float deltaTime_ = 0.0f; // 当前帧的 deltaTime

    Scene scene_;
    UiSystem* uiSystem_ = nullptr;

    Camera camera_; // 新增：摄像机实例

    bool running_ = true; // 控制主循环的内部标志
};

#endif // WINDOW_H