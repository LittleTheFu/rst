#include "RmlUiInputProcessor.h"
#include <SDL.h> // 需要 SDL_Keycode 和 SDL_SCANCODE_*

// 辅助函数：将 SDL_Keycode 映射到 Rml::Input::KeyIdentifier
// 这是一个简化的映射，对于所有键，你可能需要一个更完整的 switch-case
Rml::Input::KeyIdentifier SDLKeyToRmlKey(SDL_Keycode sdl_key)
{
    using namespace Rml::Input;
    switch (sdl_key)
    {
    case SDLK_BACKSPACE: return KI_BACK;
    case SDLK_TAB: return KI_TAB;
    case SDLK_RETURN: return KI_RETURN;
    case SDLK_ESCAPE: return KI_ESCAPE;
    case SDLK_SPACE: return KI_SPACE;
    case SDLK_KP_0: return KI_NUMPAD0;
    // ... (复制你 Window.cpp 中完整的映射逻辑) ...
    case SDLK_a: return KI_A;
    // ... 其他字母和数字 ...
    case SDLK_F1: return KI_F1;
    // ... 其他功能键和特殊键 ...
    case SDLK_LSHIFT: return KI_LSHIFT;
    case SDLK_RSHIFT: return KI_RSHIFT;
    case SDLK_LCTRL: return KI_LCONTROL;
    case SDLK_RCTRL: return KI_RCONTROL;
    case SDLK_LALT: return KI_LALT;
    case SDLK_RALT: return KI_RALT; // 注意SDL的ALT可能和RmlUi的KI_MENU冲突，需谨慎
    case SDLK_DELETE: return KI_DELETE;
    case SDLK_INSERT: return KI_INSERT;
    case SDLK_HOME: return KI_HOME;
    case SDLK_END: return KI_END;
    case SDLK_PAGEUP: return KI_PRIOR;
    case SDLK_PAGEDOWN: return KI_NEXT;
    case SDLK_UP: return KI_UP;
    case SDLK_DOWN: return KI_DOWN;
    case SDLK_LEFT: return KI_LEFT;
    case SDLK_RIGHT: return KI_RIGHT;
    default: return KI_UNKNOWN;
    }
}

// 辅助函数：获取 RmlUi 键盘修饰符
int GetRmlUiKeyModifiers()
{
    int rml_modifiers = 0;
    const Uint8* state = SDL_GetKeyboardState(NULL); // 获取当前键盘状态
    if (state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT])
        rml_modifiers |= Rml::Input::KM_SHIFT;
    if (state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL])
        rml_modifiers |= Rml::Input::KM_CTRL;
    if (state[SDL_SCANCODE_LALT] || state[SDL_SCANCODE_RALT])
        rml_modifiers |= Rml::Input::KM_ALT;
    // 对于 super key (Windows key / Command key)，SDL 没有直接的修饰符标志。
    // 如果需要，可能需要检查 SDLK_LGUI 和 SDLK_RGUI
    if (state[SDL_SCANCODE_LGUI] || state[SDL_SCANCODE_RGUI])
        rml_modifiers |= Rml::Input::KM_SUPER; // RmlUi 提供了 KM_SUPER
    return rml_modifiers;
}