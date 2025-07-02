#ifndef ABSTRACT_INPUT_PROCESSOR_H
#define ABSTRACT_INPUT_PROCESSOR_H

#include <SDL.h>    // 需要 SDL_Event
#include "commandQueue.h" // 需要 CommandQueue 来添加命令

// 抽象输入处理器接口
// 所有具体的输入处理器（ImGui, RmlUi, 游戏输入）都将实现此接口
class AbstractInputProcessor {
public:
    virtual ~AbstractInputProcessor() = default;

    // 处理单个 SDL 事件。
    // 返回 true 表示该事件已被此处理器“消费”（通常用于UI），不应再传递给链中后续处理器。
    // 返回 false 表示事件未被消费，或处理后仍可被后续处理器处理（如InputManager仅缓存状态）。
    virtual bool ProcessEvent(const SDL_Event& event, CommandQueue& commandQueue) = 0;

    // 可选：在每帧开始，SDL 事件循环前调用，用于处理器自身的帧初始化。
    // 例如，ImGui 的 NewFrame() 或 RmlUi 的 Update()。
    virtual void BeginFrame() {}

    // 查询此处理器是否希望捕获鼠标输入。
    // 例如，ImGui::GetIO().WantCaptureMouse 或 RmlUi::IsMouseInteracting()。
    // 用于高优先级UI决定是否阻止游戏鼠标输入。
    virtual bool WantsToCaptureMouse() const = 0;

    // 查询此处理器是否希望捕获键盘输入。
    // 例如，ImGui::GetIO().WantCaptureKeyboard 或 RmlUi::GetFocusElement() != nullptr。
    // 用于高优先级UI决定是否阻止游戏键盘输入。
    virtual bool WantsToCaptureKeyboard() const = 0;
};

#endif // ABSTRACT_INPUT_PROCESSOR_H