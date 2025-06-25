#include "InputManager.h"
#include <iostream> // 调试用，最终可移除

// InputManager 构造函数
InputManager::InputManager() {
    // 获取 SDL 键盘状态的内部指针。这个指针由 SDL 管理，我们只读取它的值。
    currentSDLKeyStates_ = SDL_GetKeyboardState(NULL);

    // 初始化 previousKeyStates_，假设所有键在程序开始时都未按下
    // SDL_NUM_SCANCODES 是 SDL 支持的最大扫描码数量
    for (int i = 0; i < SDL_NUM_SCANCODES; ++i) {
        previousKeyStates_[(SDL_Scancode)i] = false;
    }

    // 初始化鼠标按钮状态
    currentMouseButtonStates_[SDL_BUTTON_LEFT] = false;
    currentMouseButtonStates_[SDL_BUTTON_MIDDLE] = false;
    currentMouseButtonStates_[SDL_BUTTON_RIGHT] = false;
    // 根据需要可以添加更多按钮
    previousMouseButtonStates_ = currentMouseButtonStates_; // 初始化上一帧状态与当前帧相同
}

// InputManager 析构函数
InputManager::~InputManager() {
    // currentSDLKeyStates_ 不需要手动释放，因为它是 SDL 内部管理的指针
}

// 获取 InputManager 单例实例
InputManager& InputManager::GetInstance() {
    static InputManager instance; // 静态局部变量，保证只创建一次实例
    return instance;
}

// 每帧开始时调用，更新“上一帧”的输入状态
void InputManager::Update() {
    // 将当前键盘状态保存为上一帧状态
    previousKeyStates_.clear();
    for (int i = 0; i < SDL_NUM_SCANCODES; ++i) {
        previousKeyStates_[(SDL_Scancode)i] = (currentSDLKeyStates_[i] != 0);
    }
    // 将当前鼠标按钮状态保存为上一帧状态
    previousMouseButtonStates_ = currentMouseButtonStates_;

    // 重置鼠标移动和滚轮量，因为它们是相对量，每帧都需要清零
    mouseDeltaX_ = 0.0f;
    mouseDeltaY_ = 0.0f;
    mouseScrollY_ = 0.0f;

    // 重置退出请求标志，直到下一帧再次收到 SDL_QUIT
    quitRequested_ = false;
}

// 处理单个 SDL 事件，由 Window 类在事件循环中调用
void InputManager::ProcessEvent(const SDL_Event& event) {
    // --- 窗口事件 ---
    if (event.type == SDL_QUIT) {
        quitRequested_ = true; // 标记收到 SDL_QUIT 事件
    }

    // --- 键盘事件 ---
    // 对于键盘事件，我们主要依赖 SDL_GetKeyboardState 来获取当前状态
    // 和 previousKeyStates_ 来判断 IsKeyPressed/IsKeyReleased。
    // 所以这里不需要显式处理 SDL_KEYDOWN/SDL_KEYUP 来更新状态。
    // 如果你有其他需要事件触发的键盘逻辑，可以在这里添加。

    // --- 鼠标事件 ---
    if (event.type == SDL_MOUSEMOTION) {
        mouseDeltaX_ = static_cast<float>(event.motion.xrel);
        mouseDeltaY_ = static_cast<float>(event.motion.yrel);
    }
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        currentMouseButtonStates_[event.button.button] = true;
    }
    if (event.type == SDL_MOUSEBUTTONUP) {
        currentMouseButtonStates_[event.button.button] = false;
    }
    if (event.type == SDL_MOUSEWHEEL) {
        mouseScrollY_ = static_cast<float>(event.wheel.y); // 获取垂直滚轮量
        // event.wheel.x 可用于水平滚轮量（如果需要）
    }
}

// 查询某个键是否当前被按下 (持续性)
bool InputManager::IsKeyDown(SDL_Scancode key) const {
    return currentSDLKeyStates_[key] != 0;
}

// 查询某个键是否在当前帧被首次按下 (一次性触发)
bool InputManager::IsKeyPressed(SDL_Scancode key) const {
    return currentSDLKeyStates_[key] != 0 &&
           (!previousKeyStates_.count(key) || !previousKeyStates_.at(key));
}

// 查询某个键是否在当前帧被首次松开 (一次性触发)
bool InputManager::IsKeyReleased(SDL_Scancode key) const {
    return currentSDLKeyStates_[key] == 0 &&
           (previousKeyStates_.count(key) && previousKeyStates_.at(key));
}

// 查询某个鼠标按钮是否当前被按下 (持续性)
bool InputManager::IsMouseButtonDown(Uint8 button) const {
    return currentMouseButtonStates_.count(button) && currentMouseButtonStates_.at(button);
}

// 查询某个鼠标按钮是否在当前帧被首次按下 (一次性触发)
bool InputManager::IsMouseButtonPressed(Uint8 button) const {
    return currentMouseButtonStates_.count(button) && currentMouseButtonStates_.at(button) &&
           (!previousMouseButtonStates_.count(button) || !previousMouseButtonStates_.at(button));
}

// 查询某个鼠标按钮是否在当前帧被首次松开 (一次性触发)
bool InputManager::IsMouseButtonReleased(Uint8 button) const {
    return (!currentMouseButtonStates_.count(button) || !currentMouseButtonStates_.at(button)) &&
           (previousMouseButtonStates_.count(button) && previousMouseButtonStates_.at(button));
}