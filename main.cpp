#define SDL_MAIN_HANDLED

#include "Window.h"

int main(int argc, char* argv[]) {
    // 创建窗口对象
    Window window("OpenGL with GLAD and SDL2", 800, 600);

    // 主循环
    while (window.isRunning()) {
        window.updateFPS();  // 更新 FPS
        window.render();      // 渲染并显示 FPS
    }

    return 0;
}
