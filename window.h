#ifndef WINDOW_H
#define WINDOW_H

#include <SDL.h>
#include <glad/glad.h>
#include <chrono>
#include <iostream>

#include "scene.h"

// ImGui 头文件
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"

class Window {
public:
    Window(const char* title, int width, int height);
    ~Window();

    void updateFPS();
    void render();
    bool isRunning();

private:
    SDL_Window* window = nullptr;
    SDL_GLContext glContext;

    int fps = 0;
    int frameCount = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;

    Scene scene_;
};

#endif // WINDOW_H
