#ifndef IMGUI_INPUT_PROCESSOR_H
#define IMGUI_INPUT_PROCESSOR_H

#include "AbstractInputProcessor.h"
#include "imgui.h" // 需要 ImGui 的头文件
#include "imgui_impl_sdl2.h" // 需要 ImGui SDL2 实现的头文件

// ImGui 输入处理器
class ImGuiInputProcessor : public AbstractInputProcessor {
public:
    // 构造函数不带参数，因为ImGui的事件处理是全局的，无需特定上下文
    ImGuiInputProcessor() = default;

    // 处理 SDL 事件，由 ImGui 后端处理
    // ImGui 的事件处理通常是内部完成的，它不直接返回是否“消费”事件。
    // 而是通过 ImGui::GetIO().WantCapture* 来指示。
    // 因此，这里始终返回 false，让事件继续向下传递，
    // 但 ImGui 内部已记录了它是否需要该事件。
    bool ProcessEvent(const SDL_Event& event, CommandQueue& commandQueue) override {
        // 让 ImGui 的 SDL 后端处理事件
        // ImGui_ImplSDL2_ProcessEvent 返回 bool，但这里我们选择不完全依赖它，
        // 而是通过 WantCapture* 属性来判断优先级。
        ImGui_ImplSDL2_ProcessEvent(&event);
        return false; // ImGui通常不“消费”事件，而是通过WantCapture*表示需求
    }

    // 在每帧开始时调用 ImGui::NewFrame()
    void BeginFrame() override {
        // 在 ImGuiInputProcessor 的 BeginFrame 中调用 ImGui::NewFrame() 是合理的，
        // 确保 ImGui 在每一帧的输入处理开始前初始化其状态。
        ImGui_ImplSDL2_NewFrame();
    }

    // 查询 ImGui 是否希望捕获鼠标输入
    bool WantsToCaptureMouse() const override {
        // 检查 ImGui 是否正在处理鼠标事件（例如，鼠标悬停在窗口或控件上）
        return ImGui::GetIO().WantCaptureMouse;
    }

    // 查询 ImGui 是否希望捕获键盘输入
    bool WantsToCaptureKeyboard() const override {
        // 检查 ImGui 是否正在处理键盘事件（例如，文本输入框有焦点）
        return ImGui::GetIO().WantCaptureKeyboard;
    }
};

#endif // IMGUI_INPUT_PROCESSOR_H