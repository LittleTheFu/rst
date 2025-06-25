#pragma once
#include <SDL.h>
#include <map> // For storing key states

class InputManager {
public:
    // 单例模式的访问方法
    static InputManager& GetInstance();

    // 每帧开始时调用，更新“上一帧”的输入状态，并重置鼠标相对移动和滚轮量。
    // 不再轮询 SDL 事件。
    void Update();

    // 处理单个 SDL 事件。由 Window 类在事件循环中调用。
    void ProcessEvent(const SDL_Event& event);

    // 查询是否收到了 SDL_QUIT 事件。
    bool IsQuitRequested() const { return quitRequested_; }

    // --- 键盘查询方法 ---
    // 查询某个键是否当前被按下 (持续性按下，每帧都返回 true)
    bool IsKeyDown(SDL_Scancode key) const;
    // 查询某个键是否在当前帧被首次按下 (只在按下的那一帧返回 true)
    bool IsKeyPressed(SDL_Scancode key) const;
    // 查询某个键是否在当前帧被首次松开 (只在松开的那一帧返回 true)
    bool IsKeyReleased(SDL_Scancode key) const;

    // --- 鼠标查询方法 ---
    // 查询某个鼠标按钮是否当前被按下 (持续性按下)
    bool IsMouseButtonDown(Uint8 button) const;
    // 查询某个鼠标按钮是否在当前帧被首次按下 (只在按下的那一帧返回 true)
    bool IsMouseButtonPressed(Uint8 button) const;
    // 查询某个鼠标按钮是否在当前帧被首次松开 (只在松开的那一帧返回 true)
    bool IsMouseButtonReleased(Uint8 button) const;

    // 获取鼠标X轴的相对移动量 (自上一帧以来的变化)
    float GetMouseDeltaX() const { return mouseDeltaX_; }
    // 获取鼠标Y轴的相对移动量 (自上一帧以来的变化)
    float GetMouseDeltaY() const { return mouseDeltaY_; }
    // 获取鼠标滚轮Y轴的相对滚动量 (自上一帧以来的变化)
    float GetMouseScrollY() const { return mouseScrollY_; }

private:
    // 构造函数和析构函数私有化，禁止拷贝构造和赋值操作符，实现单例模式
    InputManager();
    ~InputManager();
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    // SDL 提供的键盘状态数组指针，直接反映当前按键的按下状态
    const Uint8* currentSDLKeyStates_;
    // 手动维护上一帧的键盘按键状态，用于检测 IsKeyPressed/IsKeyReleased
    std::map<SDL_Scancode, bool> previousKeyStates_;

    // 存储当前帧和上一帧的鼠标按键状态
    std::map<Uint8, bool> currentMouseButtonStates_;
    std::map<Uint8, bool> previousMouseButtonStates_;

    float mouseDeltaX_ = 0.0f;
    float mouseDeltaY_ = 0.0f;
    float mouseScrollY_ = 0.0f; // 鼠标滚轮垂直滚动量

    bool quitRequested_ = false; // 是否请求退出程序
};