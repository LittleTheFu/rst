#include "InputManager.h"
#include <iostream>

InputManager::InputManager()
{

    currentSDLKeyStates_ = SDL_GetKeyboardState(NULL);

    for (int i = 0; i < SDL_NUM_SCANCODES; ++i)
    {
        previousKeyStates_[(SDL_Scancode)i] = false;
    }

    currentMouseButtonStates_[SDL_BUTTON_LEFT] = false;
    currentMouseButtonStates_[SDL_BUTTON_MIDDLE] = false;
    currentMouseButtonStates_[SDL_BUTTON_RIGHT] = false;

    previousMouseButtonStates_ = currentMouseButtonStates_;
}

InputManager::~InputManager()
{
}

InputManager &InputManager::GetInstance()
{
    static InputManager instance;
    return instance;
}

void InputManager::Update()
{

    previousKeyStates_.clear();
    for (int i = 0; i < SDL_NUM_SCANCODES; ++i)
    {
        previousKeyStates_[(SDL_Scancode)i] = (currentSDLKeyStates_[i] != 0);
    }

    previousMouseButtonStates_ = currentMouseButtonStates_;

    mouseDeltaX_ = 0.0f;
    mouseDeltaY_ = 0.0f;
    mouseScrollY_ = 0.0f;

    quitRequested_ = false;
}

void InputManager::ProcessEvent(const SDL_Event &event)
{

    if (event.type == SDL_QUIT)
    {
        quitRequested_ = true;
    }

    if (event.type == SDL_MOUSEMOTION)
    {
        mouseDeltaX_ = static_cast<float>(event.motion.xrel);
        mouseDeltaY_ = static_cast<float>(event.motion.yrel);
        mouseX_ = static_cast<float>(event.motion.x);
        mouseY_ = static_cast<float>(event.motion.y);
    }
    if (event.type == SDL_MOUSEBUTTONDOWN)
    {
        currentMouseButtonStates_[event.button.button] = true;
    }
    if (event.type == SDL_MOUSEBUTTONUP)
    {
        currentMouseButtonStates_[event.button.button] = false;
    }
    if (event.type == SDL_MOUSEWHEEL)
    {
        mouseScrollY_ = static_cast<float>(event.wheel.y);
    }
}

bool InputManager::IsKeyDown(SDL_Scancode key) const
{
    return currentSDLKeyStates_[key] != 0;
}

bool InputManager::IsKeyPressed(SDL_Scancode key) const
{
    return currentSDLKeyStates_[key] != 0 &&
           (!previousKeyStates_.count(key) || !previousKeyStates_.at(key));
}

bool InputManager::IsKeyReleased(SDL_Scancode key) const
{
    return currentSDLKeyStates_[key] == 0 &&
           (previousKeyStates_.count(key) && previousKeyStates_.at(key));
}

bool InputManager::IsMouseButtonDown(Uint8 button) const
{
    return currentMouseButtonStates_.count(button) && currentMouseButtonStates_.at(button);
}

bool InputManager::IsMouseButtonPressed(Uint8 button) const
{
    return currentMouseButtonStates_.count(button) && currentMouseButtonStates_.at(button) &&
           (!previousMouseButtonStates_.count(button) || !previousMouseButtonStates_.at(button));
}

bool InputManager::IsMouseButtonReleased(Uint8 button) const
{
    return (!currentMouseButtonStates_.count(button) || !currentMouseButtonStates_.at(button)) &&
           (previousMouseButtonStates_.count(button) && previousMouseButtonStates_.at(button));
}