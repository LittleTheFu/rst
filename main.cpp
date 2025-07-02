#define SDL_MAIN_HANDLED // 确保 SDL 不尝试提供自己的 main 函数

#include "Window.h" // 你的 Window 类头文件

JPH::TempAllocator *gTempAllocator = nullptr;
JPH::JobSystem *gJobSystem = nullptr;

int main(int argc, char *argv[])
{
    JPH::RegisterDefaultAllocator();
    JPH::Factory::sInstance = new JPH::Factory(); // <--- 添加这一行！
    JPH::RegisterTypes();

    Window window("OpenGL with GLAD and SDL2", 800, 600);

    while (window.isRunning())
    {
        window.update();
        window.render();
    }

    return 0;
}
