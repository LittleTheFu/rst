// Window.cpp (修改后)
#include "Window.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// Window 构造函数
// 注意：确保 scene_ 的构造函数接收 width 和 height
Window::Window(const char* title, int width, int height)
{
    // ----------------------------------------------------
    // SDL 初始化
    // 增加 SDL_INIT_TIMER 和 SDL_INIT_GAMECONTROLLER，ImGui 可能会用到
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) < 0) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        exit(-1);
    }

    // ----------------------------------------------------
    // OpenGL 属性设置
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);       // 启用双缓冲
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);         // 设置深度缓冲区大小
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);        // 设置模板缓冲区大小
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // 推荐使用

    // ----------------------------------------------------
    // 创建 SDL 窗口
    window = SDL_CreateWindow(title,
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              width,
                              height,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window) {
        printf("Failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        exit(-1);
    }

    // ----------------------------------------------------
    // 创建 OpenGL 上下文
    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        printf("Failed to create OpenGL context: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(-1);
    }

    // 设置当前 OpenGL 上下文
    if (SDL_GL_MakeCurrent(window, glContext) != 0) {
        std::cerr << "Failed to make current context: " << SDL_GetError() << std::endl;
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        exit(-1);
    }

    // ----------------------------------------------------
    // 使用 GLAD 加载 OpenGL 函数
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(-1);
    }

    // 设置 OpenGL 视口
    int w, h;
    SDL_GL_GetDrawableSize(window, &w, &h); // 获取实际可绘制区域大小（考虑高DPI）
    glViewport(0, 0, w, h);

    // ----------------------------------------------------
    // V-Sync On/Off (0: Off, 1: On)
    SDL_GL_SetSwapInterval(0);

    // ----------------------------------------------------
    // 初始化 UiSystem <-- 新增
    uiSystem = new UiSystem(window, glContext);

    // ----------------------------------------------------
    // 其他初始化
    lastTime = std::chrono::high_resolution_clock::now();
    scene_.init(); // 初始化你的场景
}

// Window 析构函数
Window::~Window()
{
    // 清理 UiSystem <-- 新增
    delete uiSystem;
    uiSystem = nullptr;

    // ----------------------------------------------------
    // SDL 清理
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Window::updateFPS()
{
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> deltaTime = currentTime - lastTime;

    frameCount++;
    if (deltaTime.count() >= 1.0f) {
        fps = frameCount;
        frameCount = 0;
        lastTime = currentTime;

        char title[128];
        snprintf(title, sizeof(title), "OpenGL + SDL2 + ImGui - FPS: %d", fps);
        SDL_SetWindowTitle(window, title);
    }
}

void Window::render()
{
    // 1. ImGui 新帧 <-- 委托给 UiSystem
    uiSystem->NewFrame();

    // 2. 绘制自定义 UI <-- 委托给 UiSystem，并传入当前 FPS
    uiSystem->DrawUI(fps);

    // 3. 清除缓冲区
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 4. 延迟渲染逻辑
    scene_.run();

    // 5. 渲染 ImGui <-- 委托给 UiSystem
    uiSystem->Render();

    // 6. 交换窗口
    SDL_GL_SwapWindow(window);
}

bool Window::isRunning()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        uiSystem->ProcessEvent(&event); // <-- 事件转发给 UiSystem

        if (event.type == SDL_QUIT)
            return false;
        // 处理窗口大小变化事件，并传递给 Scene
        if (event.type == SDL_WINDOWEVENT) {
            if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
                event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                int w, h;
                SDL_GetWindowSize(window, &w, &h);
                glViewport(0, 0, w, h);
                // 假设你的 Scene 类有 resize 方法
                scene_.resize(w, h);
            }
        }
    }
    return true;
}