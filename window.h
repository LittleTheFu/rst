// Window.h (修改后)
#ifndef WINDOW_H
#define WINDOW_H

#include <SDL.h>
#include <glad/glad.h>
#include <chrono>
#include <iostream>

#include "scene.h"
#include "UiSystem.h" // <-- 包含新的 UiSystem 头文件

class Window {
public:
    Window(const char* title, int width, int height);
    ~Window();

    void updateFPS();
    void render();
    bool isRunning(); // 这个函数现在只处理 SDL_QUIT，其他事件将通过 main 循环直接传递

private:
    SDL_Window* window = nullptr;
    SDL_GLContext glContext;

    int fps = 0;
    int frameCount = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;

    Scene scene_;
    UiSystem* uiSystem = nullptr; // <-- 添加 UiSystem 成员变量
};

#endif // WINDOW_H