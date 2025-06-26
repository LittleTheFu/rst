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
    ImGui::Begin("Debug Window");
    ImGui::Text("Average FPS: %d", currentFPS);
    ImGui::Checkbox("Show ImGui Demo Window", &showDemoWindow);

    if (ImGui::Button("Capture"))
    {
        std::cout << "Capture button pressed!" << std::endl;
        if (onCaptureButtonClicked) {
            onCaptureButtonClicked();
        }
    }

    if (ImGui::Button("Toggle Debug Draw"))
    {
        std::cout << "Toggle Debug Draw button pressed!" << std::endl;
        if (onToggleDebugButtonClicked) {
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
            std::cout << "Selected mode: " << name << " (value " << value << ")" << std::endl;
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
    if (ImGui::SliderFloat("SSR Weight", &ssrWeight, 0.0f, max, "%.2f")) {
        std::cout << "SSR weight updated: " << ssrWeight << std::endl;
        if (onSsrWeightBarChanged) {
            onSsrWeightBarChanged(ssrWeight);
        }
    }

    if (ImGui::SliderFloat("IBL Weight", &iblWeight, 0.0f, max, "%.2f")) {
        std::cout << "IBL weight updated: " << iblWeight << std::endl;
        if (onIblWeightBarChanged) {
            onIblWeightBarChanged(iblWeight);
        }
    }

    if (ImGui::SliderFloat("Light Weight", &lightWeight, 0.0f, max, "%.2f")) {
        std::cout << "Light weight updated: " << lightWeight << std::endl;
        if (onLightWeightBarChanged) {
            onLightWeightBarChanged(lightWeight);
        }
    }

    if (ImGui::SliderFloat("OIT Weight", &oitWeight, 0.0f, max, "%.2f")) {
        std::cout << "OIT weight updated: " << oitWeight << std::endl;
        if (onOitWeightBarChanged) {
            onOitWeightBarChanged(oitWeight);
        }
    }

    if (ImGui::SliderFloat("God Ray Weight", &godRayWeight, 0.0f, max, "%.2f")) {
        std::cout << "God ray weight updated: " << godRayWeight << std::endl;
        if (onGodRayWeightBarChanged) {
            onGodRayWeightBarChanged(godRayWeight);
        }
    }

    if (ImGui::SliderFloat("Focus Distance", &focusDistance, 0.0f, 20.0f, "%.2f")) {
        std::cout << "Focus distance updated: " << focusDistance << std::endl;
        if (onFocusDistanceBarChanged) {
            onFocusDistanceBarChanged(focusDistance);
        }
    }

    ImGui::End();

    //-----properties of the selected object------
    ImGui::Begin("Selected Object Properties");
    if (onGetSelectedMeshName)
    {
        // 调用回调函数，获取选中物体的名称
        std::string selectedName = onGetSelectedMeshName();

        // 在 ImGui 中显示名称
        ImGui::Text("Selected Object Name: %s", selectedName.c_str());
    }
    else
    {
        // 如果回调没有被设置，显示一个错误或提示信息
        ImGui::Text("Selected Object Name: (Callback not set)");
    }
    ImGui::Text("properties");
    ImGui::Text("position");
    ImGui::Text("scale");
    ImGui::Text("rotation");
    ImGui::End();
    //--------------------------------------------

    if (showDemoWindow)
    {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }
}
