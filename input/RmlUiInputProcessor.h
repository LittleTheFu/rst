#ifndef RMLUI_INPUT_PROCESSOR_H
#define RMLUI_INPUT_PROCESSOR_H

#include "AbstractInputProcessor.h"
#include <RmlUi/Core.h> // RmlUi 核心头文件
#include <RmlUi/Debugger.h> // 可能需要调试器，或者可以移除

// 辅助函数：将 SDL_Keycode 映射到 Rml::Input::KeyIdentifier
// 这个函数应该从 Window.cpp 中移动到这里或者一个独立的工具文件
extern Rml::Input::KeyIdentifier SDLKeyToRmlKey(SDL_Keycode sdl_key);
// 辅助函数：获取 RmlUi 键盘修饰符
extern int GetRmlUiKeyModifiers();

// RmlUi 输入处理器
class RmlUiInputProcessor : public AbstractInputProcessor {
public:
    // 构造函数需要 Rml::Context 的引用，以便处理事件
    RmlUiInputProcessor(Rml::Context* context) : context_(context) {}

    // 处理 SDL 事件，转发给 RmlUi 上下文
    bool ProcessEvent(const SDL_Event& event, CommandQueue& commandQueue) override {
        if (!context_) return false; // 如果 context 无效，则不处理

        bool event_consumed = false; // RmlUi 是否消费了事件

        switch (event.type) {
            case SDL_MOUSEMOTION:
                // RmlUi 鼠标移动事件处理
                context_->ProcessMouseMove(event.motion.x, event.motion.y, GetRmlUiKeyModifiers());
                event_consumed = context_->IsMouseInteracting(); // 如果鼠标在RmlUi元素上，则可能被消费
                break;
            case SDL_MOUSEBUTTONDOWN:
                // RmlUi 鼠标按钮按下事件处理
                event_consumed = context_->ProcessMouseButtonDown(event.button.button - 1, true);
                break;
            case SDL_MOUSEBUTTONUP:
                // RmlUi 鼠标按钮抬起事件处理
                event_consumed = context_->ProcessMouseButtonDown(event.button.button - 1, false);
                break;
            case SDL_MOUSEWHEEL:
                // RmlUi 鼠标滚轮事件处理
                event_consumed = context_->ProcessMouseWheel(event.wheel.y, GetRmlUiKeyModifiers());
                break;
            case SDL_KEYDOWN:
                // RmlUi 键盘按下事件处理
                event_consumed = context_->ProcessKeyDown(SDLKeyToRmlKey(event.key.keysym.sym), GetRmlUiKeyModifiers());
                break;
            case SDL_KEYUP:
                // RmlUi 键盘抬起事件处理
                event_consumed = context_->ProcessKeyUp(SDLKeyToRmlKey(event.key.keysym.sym), GetRmlUiKeyModifiers());
                break;
            case SDL_TEXTINPUT:
                // RmlUi 文本输入事件处理
                event_consumed = context_->ProcessTextInput(event.text.text);
                break;
            // 其他 SDL 事件类型（如窗口事件）RmlUi 可能也需要，但通常不被“消费”
            case SDL_WINDOWEVENT:
                // 处理窗口焦点事件，RmlUi可能通过ProcessKey来模拟
                if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                    context_->ProcessKeyDown(Rml::Input::KI_HOME, 0);
                } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                    context_->ProcessKeyDown(Rml::Input::KI_END, 0);
                }
                break;
            default:
                break;
        }

        // 重要：如果 RmlUi 内部有事件监听器触发了游戏命令（例如按钮点击），
        // 那么在 RmlUi 事件监听器的回调函数中，可以直接调用 commandQueue.AddCommand()。
        // 例如：
        // closeButton->AddEventListener(Rml::EventId::Click, [&commandQueue](Rml::Event& event) {
        //     commandQueue.AddCommand(std::make_unique<QuitApplicationCommand>());
        // });

        return event_consumed; // 返回 RmlUi 是否消费了此事件
    }

    // 在每帧开始时调用 RmlUi 的更新方法
    void BeginFrame() override {
        if (context_) {
            context_->Update();
        }
    }

    // 查询 RmlUi 是否希望捕获鼠标输入
    bool WantsToCaptureMouse() const override {
        // RmlUi 通过 IsMouseInteracting() 判断鼠标是否在UI元素上
        return context_ ? context_->IsMouseInteracting() : false;
    }

    // 查询 RmlUi 是否希望捕获键盘输入
    bool WantsToCaptureKeyboard() const override {
        // RmlUi 通过 GetFocusElement() 判断是否有元素获取了键盘焦点
        return context_ ? (context_->GetFocusElement() != nullptr) : false;
    }

private:
    Rml::Context* context_; // RmlUi 上下文指针
};

#endif // RMLUI_INPUT_PROCESSOR_H