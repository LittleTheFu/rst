#define SDL_MAIN_HANDLED // 确保 SDL 不尝试提供自己的 main 函数

#include "Window.h" // 你的 Window 类头文件

JPH::TempAllocator *gTempAllocator = nullptr;
JPH::JobSystem *gJobSystem = nullptr;

int main(int argc, char *argv[])
{
    // 1. 注册 Jolt 的默认分配器
    // 这一步必须在任何 Jolt 对象创建或任何 Jolt 内部会触发内存分配的函数调用之前执行。
    JPH::RegisterDefaultAllocator();

    // 2. 初始化其他 Jolt 全局系统（如果你需要它们）
    // 这些通常在分配器设置好之后进行。
    // 例如，如果你使用了 Jolt 的 JobSystem 或 TempAllocator：
    // gTempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024); // 10MB 临时分配器
    // gJobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1); // 或根据你的 CPU 核心数设置
    JPH::Factory::sInstance = new JPH::Factory(); // <--- 添加这一行！


    // 3. 注册所有 Jolt 物理类型
    // 这会将 Body、BoxShape 等类注册到 Jolt 的 Factory 中，以便你可以通过名称创建它们。
    JPH::RegisterTypes();

    // 创建窗口对象。注意这里将初始宽高传递给 Window 构造函数，Camera 会使用它们计算宽高比。
    Window window("OpenGL with GLAD and SDL2", 800, 600);

    // 主游戏循环
    while (window.isRunning())
    {
        // 更新所有游戏逻辑和输入
        window.update();
        // 渲染场景和 UI
        window.render();
    }

    // 程序结束，Window 析构函数会自动清理 SDL 和 OpenGL 资源
    return 0;
}
