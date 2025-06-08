// UiSystem.h
#ifndef UI_SYSTEM_H
#define UI_SYSTEM_H

#include <SDL.h>
#include <glad/glad.h>

// ImGui 头文件
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"
#include <functional>
#include "uiSceneData.h"

// 引入你的 Scene 类，如果 UiSystem 需要访问 Scene 的参数
// #include "scene.h" // 如果需要，请取消注释

class UiSystem {
public:
    std::function<void(bool)> onBlurClicked;
    bool isBlurOn = false;

public:
    UiSceneData uiSceneData;
    int selectedRenderMode = 0;

public:
    std::function<void(float)> onSsrWeightBarChanged;
    float ssrWeight = 0.0f;

    std::function<void(float)> onIblWeightBarChanged;
    float iblWeight = 0.0f;

    std::function<void(float)> onLightWeightBarChanged;
    float lightWeight = 0.0f;

    std::function<void(float)> onOitWeightBarChanged;
    float oitWeight = 0.0f;

    std::function<void(float)> onGodRayWeightBarChanged;
    float godRayWeight = 0.0f;
    

public:
    // 构造函数：初始化 ImGui 后端
    // 传入 SDL 窗口和 OpenGL 上下文
    UiSystem(SDL_Window* window, SDL_GLContext glContext);

    // 析构函数：清理 ImGui 资源
    ~UiSystem();

    // 每一帧开始时调用，准备 ImGui 渲染
    void NewFrame();

    // 渲染 ImGui 绘制数据
    void Render();

    // 将 SDL 事件转发给 ImGui
    void ProcessEvent(SDL_Event* event);

    // 绘制自定义 ImGui 界面。
    // 你可以传入需要显示或控制的数据，例如 FPS。
    // 如果需要控制 Scene 参数，可以传入 Scene* 或相关参数。
    void DrawUI(int currentFPS); // 示例：传入当前 FPS

    // 示例：可以添加控制 UI 窗口是否显示的标志
    bool showDemoWindow = false;

    // 示例：可以添加一些将来在 UI 中控制的变量
    // float exampleFloat = 0.5f;
    // float exampleColor[3] = { 1.0f, 0.5f, 0.0f };

private:
    // ImGui 上下文是在 ImGui::CreateContext() 中全局设置的，
    // 因此这里不需要显式的 ImGuiContext 成员。
    // 这里可以放 ImGuiIO 的引用或者指针，以便快速访问，但不是必须的。
    // ImGuiIO& io_; // 如果你需要缓存
};

#endif // UI_SYSTEM_H