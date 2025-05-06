#include "Window.h"
#include <stdio.h>
#include <stdlib.h>

Window::Window(const char* title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        exit(-1);
    }

    // 设置 OpenGL 版本
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // 创建窗口
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        exit(-1);
    }

    // 创建 OpenGL 上下文
    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        printf("Failed to create OpenGL context: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(-1);
    }

    // 使用 GLAD 加载 OpenGL 函数
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(-1);
    }

    lastTime = std::chrono::high_resolution_clock::now();  // 初始化时间戳

    scene_.init();
}

Window::~Window() {
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Window::updateFPS() {
    // 获取当前时间
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> deltaTime = currentTime - lastTime;

    // 更新 FPS 计数
    frameCount++;
    if (deltaTime.count() >= 1.0f) {  // 每秒刷新一次 FPS
        fps = frameCount;
        frameCount = 0;
        lastTime = currentTime;

        // 更新窗口标题为当前 FPS
        char title[128];
        snprintf(title, sizeof(title), "OpenGL with GLAD and SDL2 - FPS: %d", fps);
        SDL_SetWindowTitle(window, title);  // 更新窗口标题
    }
}

void Window::render() {
    // 清屏操作
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    scene_.run();

    // 交换窗口
    SDL_GL_SwapWindow(window);
}

bool Window::isRunning() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }
    }
    return true;
}
