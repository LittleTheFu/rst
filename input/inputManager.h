#pragma once
#include <SDL.h>
#include <map>

class InputManager
{
public:
    static InputManager &GetInstance();

    void Update();

    void ProcessEvent(const SDL_Event &event);

    bool IsQuitRequested() const { return quitRequested_; }

    bool IsKeyDown(SDL_Scancode key) const;

    bool IsKeyPressed(SDL_Scancode key) const;

    bool IsKeyReleased(SDL_Scancode key) const;

    bool IsMouseButtonDown(Uint8 button) const;

    bool IsMouseButtonPressed(Uint8 button) const;

    bool IsMouseButtonReleased(Uint8 button) const;

    float GetMouseX() const { return mouseX_; }

    float GetMouseY() const { return mouseY_; }

    float GetMouseDeltaX() const { return mouseDeltaX_; }

    float GetMouseDeltaY() const { return mouseDeltaY_; }

    float GetMouseScrollY() const { return mouseScrollY_; }

private:
    InputManager();
    ~InputManager();
    InputManager(const InputManager &) = delete;
    InputManager &operator=(const InputManager &) = delete;

    const Uint8 *currentSDLKeyStates_;

    std::map<SDL_Scancode, bool> previousKeyStates_;

    std::map<Uint8, bool> currentMouseButtonStates_;
    std::map<Uint8, bool> previousMouseButtonStates_;

    float mouseX_ = 0.0f;
    float mouseY_ = 0.0f;
    float mouseDeltaX_ = 0.0f;
    float mouseDeltaY_ = 0.0f;
    float mouseScrollY_ = 0.0f;

    bool quitRequested_ = false;
};