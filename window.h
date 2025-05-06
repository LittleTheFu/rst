#ifndef WINDOW_H
#define WINDOW_H

#include <SDL.h>
#include <glad/glad.h>
#include <chrono>

class Window {
public:
    // 构造函数：初始化窗口和 OpenGL 上下文
    Window(const char* title, int width, int height);

    // 析构函数：清理资源
    ~Window();

    // 更新 FPS 计数
    void updateFPS();

    // 渲染内容（清屏并显示 FPS）
    void render();

    // 事件循环判断窗口是否仍在运行
    bool isRunning();

private:
    SDL_Window* window = nullptr;          // SDL 窗口对象
    SDL_GLContext glContext;               // OpenGL 上下文
    int fps = 0;                           // 当前 FPS
    int frameCount = 0;                    // 帧计数器
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;  // 上一次时间戳
};

#endif // WINDOW_H
