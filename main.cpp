#define SDL_MAIN_HANDLED // 确保 SDL 不尝试提供自己的 main 函数

#include "Window.h" // 你的 Window 类头文件
// InputManager 和 Camera 已经包含在 Window.h 中，这里不需要重复包含

int main(int argc, char* argv[]) {
    // 创建窗口对象。注意这里将初始宽高传递给 Window 构造函数，Camera 会使用它们计算宽高比。
    Window window("OpenGL with GLAD and SDL2", 800, 600);

    // 主游戏循环
    while (window.isRunning()) {
        // 更新所有游戏逻辑和输入
        window.update();
        // 渲染场景和 UI
        window.render();
    }

    // 程序结束，Window 析构函数会自动清理 SDL 和 OpenGL 资源
    return 0;
}