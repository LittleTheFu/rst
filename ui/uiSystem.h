// UiSystem.h
#ifndef UI_SYSTEM_H
#define UI_SYSTEM_H

#include <SDL.h>
#include <glad/glad.h>

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"
#include <functional>
#include "uiSceneData.h"


class UiSystem {
public:
    UiSceneData uiSceneData;
    int selectedRenderMode = 0;

public:
    std::function<void(void)> onCaptureButtonClicked;
    std::function<void(void)> onToggleDebugButtonClicked;

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

    std::function<void(float)> onFocusDistanceBarChanged;
    float focusDistance = 0.0f;

    std::function<Mesh*()> onGetSelectedMesh;
    
public:
    UiSystem(SDL_Window* window, SDL_GLContext glContext);
    ~UiSystem();

    void NewFrame();
    void Render();
    void ProcessEvent(SDL_Event* event);
    void DrawUI(int currentFPS); // 示例：传入当前 FPS

private:
    bool showDemoWindow = false;
};

#endif // UI_SYSTEM_H