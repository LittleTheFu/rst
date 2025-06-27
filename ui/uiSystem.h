#ifndef UI_SYSTEM_H
#define UI_SYSTEM_H

#include <SDL.h>
#include <glad/glad.h>

#include "imgui.h"
#include <functional>
#include "uiSceneData.h" // UiSceneData 也要更新
#include "sceneObject.h" // 引入 ISceneObject 接口

class UiSystem {
public:
    UiSceneData uiSceneData; // UiSceneData 内部的结构也需要适配 ISceneObject
    int selectedRenderMode = 0;

public:
    // 按钮回调
    std::function<void(void)> onCaptureButtonClicked;
    std::function<void(void)> onToggleDebugButtonClicked;

    // 滑块回调和对应的值
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

    // !!! 关键改动 !!!
    // 替换 onGetSelectedMesh 为 onGetSelectedObject，并返回 ISceneObject*
    std::function<ISceneObject*()> onGetSelectedObject;
    
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