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

// // main.cpp
// #include <iostream>
// #include <chrono> // 用于高精度时钟
// #include <thread> // 用于线程暂停

// // 包含你的物理系统头文件
// #include "physicsSystem.h"

// // --- 最小模拟 ISceneObject 和 SceneData 用于编译 ---
// // 在真实项目中，这些将是你实际的渲染/场景类。
// // 这里重新加入了它们，以确保 main.cpp 可以独立编译并运行。

// // // 模拟 ISceneObject
// // class ISceneObject {
// // public:
// //     virtual ~ISceneObject() = default;
// //     virtual void setModelMatrix(const Eigen::Matrix4f& matrix) {
// //         mModelMatrix = matrix;
// //         // 在真实引擎中，这将更新 GPU 缓冲区或场景图节点
// //         // 用于测试，我们只存储它。
// //     }
// //     virtual Eigen::Matrix4f getModelMatrix() const { return mModelMatrix; }
// //     virtual Eigen::Vector3f getScale() const { return Eigen::Vector3f(1.0f, 1.0f, 1.0f); } // 默认缩放
// //     virtual std::string getName() const { return "MockSceneObject"; } // 用于日志记录
// //     virtual void setPosition(const Eigen::Vector3f& pos) { mModelMatrix.block<3,1>(0,3) = pos; }
// //     virtual void setRotation(const Eigen::Quaternionf& rot) { mModelMatrix.block<3,3>(0,0) = rot.toRotationMatrix() * Eigen::DiagonalMatrix<float, 3>(getScale()); }
// // private:
// //     Eigen::Matrix4f mModelMatrix = Eigen::Matrix4f::Identity();
// // };

// // // 模拟 SceneData (如果 PhysicsSystem 需要，否则可以删除)
// // class SceneData {
// // public:
// //     // 如果编译需要，可能添加一些虚拟成员或函数
// // };

// // 模拟 debug_utils (用于 GL_CHECK_ERROR 和你的日志记录)
// #define GL_CHECK_ERROR() ((void)0) // 虚拟宏
// // 如果你有特定的日志宏，请在此处定义：
// // #define LOG_INFO(...) std::cout << __VA_ARGS__ << std::endl;

// // --- 模拟类结束 ---

// int main() {
//     std::cout << "Starting Jolt Physics test application." << std::endl;

//     // 创建 PhysicsSystem 实例
//     PhysicsSystem myPhysicsSystem;

//     // 1. 初始化物理系统
//     myPhysicsSystem.Init();

//     const float cTargetFPS = 60.0f; // 目标帧率 60 FPS
//     // **重要**: 物理更新通常使用固定时间步长。这里我们将这个固定步长传递给 PhysicsSystem::Update
//     const float cPhysicsFixedTimestep = 1.0f / cTargetFPS; // Jolt 内部也可能使用自己的固定时间步长
//     const int cTotalSimulationTimeSeconds = 5; // 模拟 5 秒
//     const int cNumSimulationFrames = static_cast<int>(cTotalSimulationTimeSeconds * cTargetFPS);

//     auto lastFrameTime = std::chrono::high_resolution_clock::now();

//     for (int frame = 0; frame < cNumSimulationFrames; ++frame) {
//         auto currentTime = std::chrono::high_resolution_clock::now();
//         // 计算自上次帧以来的实际经过时间，这可以用来调整睡眠时间，让主循环接近目标帧率。
//         // 但对于物理更新，我们直接使用固定的 cPhysicsFixedTimestep。
//         float actualFrameDeltaTime = std::chrono::duration<float>(currentTime - lastFrameTime).count();
//         lastFrameTime = currentTime;

//         // 避免 deltaTime 过大导致物理不稳定，通常在游戏循环中这样做。
//         // 对于 Jolt PhysicsSystem::Update 内部，它已经处理了多步模拟来跟上大的 deltaTime。
//         // 所以这里我们直接传递一个期望的物理更新步长。
//         if (actualFrameDeltaTime > 0.1f) actualFrameDeltaTime = 0.1f; 

//         // 2. 更新物理模拟
//         // 传递一个固定的时间步长，让物理系统知道每一步应该前进多少时间。
//         // PhysicsSystem::Update 内部会根据这个传入的时间步长和自身的固定步长进行多步或单步模拟。
//         myPhysicsSystem.Update(cPhysicsFixedTimestep); // 直接使用固定的物理更新步长

//         // 模拟游戏循环延迟以维持目标帧率
//         // 这里我们尝试让主循环的帧率接近 cTargetFPS。
//         auto endPhysicsUpdateTime = std::chrono::high_resolution_clock::now();
//         std::chrono::duration<float> timeSpentInFrame = endPhysicsUpdateTime - currentTime;
        
//         // 计算还需要等待多久才能达到目标帧时间
//         std::chrono::duration<float> sleepDuration = std::chrono::duration<float>(cPhysicsFixedTimestep) - timeSpentInFrame; // 使用物理固定步长作为目标帧时间

//         // --- 强制引入一个最小延迟，以确保控制台有时间打印输出 ---
//         // 这对于调试非常有用，否则球体可能在几毫秒内落地休眠，你看不到中间过程。
//         const std::chrono::milliseconds minSleepTime(10); // 至少暂停 10 毫秒
//         if (sleepDuration < minSleepTime) {
//             sleepDuration = minSleepTime;
//         }

//         if (sleepDuration.count() > 0) {
//             std::this_thread::sleep_for(sleepDuration);
//         }
//     }

//     // 3. 关闭物理系统
//     myPhysicsSystem.Shutdown();

//     std::cout << "Jolt Physics test application finished." << std::endl;

//     return 0;
// }