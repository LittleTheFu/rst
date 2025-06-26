#include "UiSystem.h"
#include <iostream>

UiSystem::UiSystem(SDL_Window *window, SDL_GLContext glContext)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 460");
}

UiSystem::~UiSystem()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void UiSystem::NewFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void UiSystem::Render()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UiSystem::ProcessEvent(SDL_Event *event)
{
    ImGui_ImplSDL2_ProcessEvent(event);
}

void UiSystem::DrawUI(int currentFPS)
{
    ImGui::Begin("调试窗口");
    ImGui::Text("应用平均 FPS: %d", currentFPS);
    ImGui::Checkbox("显示 ImGui Demo 窗口", &showDemoWindow);

    if (ImGui::Button("capture"))
    {
        std::cout << "capture button pressed!!!" << std::endl;
        if(onCaptureButtonClicked) {
            onCaptureButtonClicked(); 
        }
    }

    if (ImGui::Button("toggle debug draw"))
    {
        std::cout << "toggle debug draw button pressed!!!" << std::endl;
        if(onToggleDebugButtonClicked) {
            onToggleDebugButtonClicked(); 
        }
    }

    for (size_t i = 0; i < uiSceneData.sceneData.size(); ++i)
    {
        const auto &item = uiSceneData.sceneData[i];
        const std::string &name = std::get<0>(item);
        int value = std::get<1>(item);

        if (name == "")
        {
            ImGui::NewLine();
            continue;
        }

        ImGui::PushID(static_cast<int>(i));
        if (ImGui::RadioButton(name.c_str(), &selectedRenderMode, value))
        {
            std::cout << "选择了模式: " << name << " (值 " << value << ")" << std::endl;
            if (uiSceneData.onRenderModeChanged)
            {
                uiSceneData.onRenderModeChanged(selectedRenderMode);
            }
        }
        ImGui::PopID();
        ImGui::SameLine();
    }
    ImGui::NewLine();

    float max = 5.0f;
    if (ImGui::SliderFloat("ssr weight", &ssrWeight, 0.0f, max, "%.2f")) {
        std::cout << "ssr weight！新值: " << ssrWeight << std::endl;
        if (onSsrWeightBarChanged) {
            onSsrWeightBarChanged(ssrWeight);
        }
    }

    if (ImGui::SliderFloat("ibl weight", &iblWeight, 0.0f, max, "%.2f")) {
        std::cout << "ibl weight！新值: " << iblWeight << std::endl; 
        if (onIblWeightBarChanged) {
            onIblWeightBarChanged(iblWeight);
        }
    }

    if (ImGui::SliderFloat("light weight", &lightWeight, 0.0f, max, "%.2f")) {
        std::cout << "light weight！新值: " << lightWeight << std::endl; 
        if (onLightWeightBarChanged) {
            onLightWeightBarChanged(lightWeight);
        }
    }

    if (ImGui::SliderFloat("oit weight", &oitWeight, 0.0f, max, "%.2f")) {
        std::cout << "oit weight！新值: " << oitWeight << std::endl;
        if (onOitWeightBarChanged) {
            onOitWeightBarChanged(oitWeight); 
        } 
    }

    if (ImGui::SliderFloat("god ray weight", &godRayWeight, 0.0f, max, "%.2f")) {
        std::cout << "god ray weight！新值: " << godRayWeight << std::endl;
        if (onGodRayWeightBarChanged) {
            onGodRayWeightBarChanged(godRayWeight);
        } 
    }

    if(ImGui::SliderFloat("focus distance", &focusDistance, 0.0f, 20.0f, "%.2f")) {
        std::cout << "focus distance！新值: " << focusDistance << std::endl;
        if (onFocusDistanceBarChanged) {
            onFocusDistanceBarChanged(focusDistance);
        }
    }

    ImGui::End();

    if (showDemoWindow)
    {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }
}