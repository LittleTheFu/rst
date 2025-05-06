#define SDL_MAIN_HANDLED

#include <SDL.h>
#include <glad/glad.h>  // 使用 GLAD 加载 OpenGL 函数
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        return -1;
    }

    // 设置 OpenGL 版本
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // 创建窗口
    SDL_Window* window = SDL_CreateWindow("OpenGL with GLAD and SDL2",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // 创建 OpenGL 上下文
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        printf("Failed to create OpenGL context: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // 使用 GLAD 加载 OpenGL 函数
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // 获取 OpenGL 版本
    const GLubyte* versionStr = glGetString(GL_VERSION);
    printf("OpenGL Version String: %s\n", versionStr);

    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    printf("OpenGL Version Major: %d, Minor: %d\n", major, minor);

    // 主循环（这里简单清除屏幕，渲染一个颜色）
    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // 清屏操作
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 交换窗口
        SDL_GL_SwapWindow(window);
    }

    // 清理
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
