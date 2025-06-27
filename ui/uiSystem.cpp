#include "uiSystem.h"
#include <iostream>
// Add this include!
#include <backends/imgui_impl_opengl3.h>
// Also include the SDL2 backend for completeness, though it might be transitive
#include <backends/imgui_impl_sdl2.h>
// #include <mesh.h> // 不再直接使用 Mesh，因为我们现在用 ISceneObject

// 假设 ISceneObject 提供了 getName(), getPosition(), getRotation(), getScale(), getMaterial() 等方法
// 以及 setPosition(), setRotation(), setScale() 等设置方法。
// 并且 getMaterial() 返回 std::shared_ptr<Material>
// Material 类需要提供 getName(), getAlbedoColor(), getRoughnessFactor(), getMetallicFactor()
// 以及 setAlbedoColor(), setRoughnessFactor(), setMetallicFactor()

// 假设 math.h 提供了 M_PI
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

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
        if (onCaptureButtonClicked)
        {
            onCaptureButtonClicked();
        }
    }

    if (ImGui::Button("Toggle Debug Draw"))
    {
        std::cout << "Toggle Debug Draw button pressed!" << std::endl;
        if (onToggleDebugButtonClicked)
        {
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
    if (ImGui::SliderFloat("SSR Weight", &ssrWeight, 0.0f, max, "%.2f"))
    {
        std::cout << "SSR weight updated: " << ssrWeight << std::endl;
        if (onSsrWeightBarChanged)
        {
            onSsrWeightBarChanged(ssrWeight);
        }
    }

    if (ImGui::SliderFloat("IBL Weight", &iblWeight, 0.0f, max, "%.2f"))
    {
        std::cout << "IBL weight updated: " << iblWeight << std::endl;
        if (onIblWeightBarChanged)
        {
            onIblWeightBarChanged(iblWeight);
        }
    }

    if (ImGui::SliderFloat("Light Weight", &lightWeight, 0.0f, max, "%.2f"))
    {
        std::cout << "Light weight updated: " << lightWeight << std::endl;
        if (onLightWeightBarChanged)
        {
            onLightWeightBarChanged(lightWeight);
        }
    }

    if (ImGui::SliderFloat("OIT Weight", &oitWeight, 0.0f, max, "%.2f"))
    {
        std::cout << "OIT weight updated: " << oitWeight << std::endl;
        if (onOitWeightBarChanged)
        {
            onOitWeightBarChanged(oitWeight);
        }
    }

    if (ImGui::SliderFloat("God Ray Weight", &godRayWeight, 0.0f, max, "%.2f"))
    {
        std::cout << "God ray weight updated: " << godRayWeight << std::endl;
        if (onGodRayWeightBarChanged)
        {
            onGodRayWeightBarChanged(godRayWeight);
        }
    }

    if (ImGui::SliderFloat("Focus Distance", &focusDistance, 0.0f, 20.0f, "%.2f"))
    {
        std::cout << "Focus distance updated: " << focusDistance << std::endl;
        if (onFocusDistanceBarChanged)
        {
            onFocusDistanceBarChanged(focusDistance);
        }
    }

    ImGui::End();

    // ---------- 新增：场景大纲视图窗口 ----------
    ImGui::Begin("Scene Outliner");

    // !!! 关键改动 !!!
    // 获取当前被选中的 ISceneObject (用于高亮显示)
    ISceneObject *currentSelectedObject = nullptr;
    if (onGetSelectedObject)
    { // onGetSelectedObject 是 UiSystem 的成员
        currentSelectedObject = onGetSelectedObject();
    }

    // 遍历所有 ISceneObject 并在列表中显示
    // uiSceneData.allMeshes 应该改为 uiSceneData.allSceneObjects 或类似
    // 假设 uiSceneData.allSceneObjects 现在存储的是 std::vector<ISceneObject*>
    if (!uiSceneData.allSceneObjects.empty())
    {
        for (size_t i = 0; i < uiSceneData.allSceneObjects.size(); ++i)
        {
            ISceneObject *obj = uiSceneData.allSceneObjects[i];
            if (!obj)
                continue; // 安全检查，防止空指针

            // 设置选中状态：如果当前 Object 是被选中的，则 ImGui::Selectable 会高亮显示
            bool isSelected = (obj == currentSelectedObject);

            // 为每个 Selectable 加上唯一的 ID
            ImGui::PushID(obj); // 使用 ISceneObject* 地址作为 ID
            if (ImGui::Selectable(obj->getName().c_str(), isSelected))
            {
                // 如果用户点击了这个列表项，并且我们有设置选中 Object 的回调
                // uiSceneData.onMeshSelectedFromUI 应该改为 uiSceneData.onObjectSelectedFromUI
                if (uiSceneData.onObjectSelectedFromUI)
                { 
                    uiSceneData.onObjectSelectedFromUI(obj); // 通知 Scene 选中了这个 ISceneObject
                }
            }
            ImGui::PopID(); // 匹配 PushID
        }
    }
    else
    {
        ImGui::Text("No objects in scene.");
    }

    ImGui::End(); // End Scene Outliner Window

    // --- 选中物体属性窗口 ---
    ImGui::Begin("Selected Object Properties");

    // !!! 关键改动 !!!
    // 使用 onGetSelectedObject 获取 ISceneObject*
    ISceneObject *selectedObject = nullptr;
    if (onGetSelectedObject) {
        selectedObject = onGetSelectedObject();
    }
    
    if (selectedObject)
    { 
        // 显示名称 (Text 控件，不编辑)
        ImGui::Text("Selected Object Name: %s", selectedObject->getName().c_str());

        ImGui::Separator();

        // -------------------------------------------------------------------
        // 编辑位置 (Position)
        // -------------------------------------------------------------------
        Eigen::Vector3f currentPos = selectedObject->getPosition();
        float pos[3] = {currentPos.x(), currentPos.y(), currentPos.z()};

        if (ImGui::InputFloat3("Position", pos))
        {
            selectedObject->setPosition(Eigen::Vector3f(pos[0], pos[1], pos[2]));
        }

        // -------------------------------------------------------------------
        // 编辑旋转 (Rotation) - 使用欧拉角
        // -------------------------------------------------------------------
        Eigen::Quaternionf currentRotQuat = selectedObject->getRotation();
        Eigen::Vector3f currentEuler = currentRotQuat.toRotationMatrix().eulerAngles(0, 1, 2) * 180.0f / M_PI;
        float rot[3] = {currentEuler.x(), currentEuler.y(), currentEuler.z()};

        if (ImGui::InputFloat3("Rotation (Euler)", rot))
        {
            Eigen::Quaternionf newRotQuat =
                Eigen::AngleAxisf(rot[2] * M_PI / 180.0f, Eigen::Vector3f::UnitZ()) * Eigen::AngleAxisf(rot[1] * M_PI / 180.0f, Eigen::Vector3f::UnitY()) * Eigen::AngleAxisf(rot[0] * M_PI / 180.0f, Eigen::Vector3f::UnitX()); 

            selectedObject->setRotation(newRotQuat);
        }

        // -------------------------------------------------------------------
        // 编辑缩放 (Scale)
        // -------------------------------------------------------------------
        Eigen::Vector3f currentScale = selectedObject->getScale();
        float scale[3] = {currentScale.x(), currentScale.y(), currentScale.z()};

        if (ImGui::InputFloat3("Scale", scale))
        {
            selectedObject->setScale(Eigen::Vector3f(scale[0], scale[1], scale[2]));
        }

        ImGui::Separator(); 
        ImGui::Text("Material Properties:");

        // 尝试获取选中 Object 的材质
        // std::shared_ptr<Material> material = selectedObject->getMaterial();

        // if (material)
        // { 
        //     // 显示材质名称
        //     ImGui::Text("Material Name: %s", material->getName().c_str());

        //     // -------------------------------------------------------------
        //     // 编辑 Albedo 颜色
        //     // -------------------------------------------------------------
        //     Eigen::Vector3f albedoColor = material->getAlbedoColor();
        //     float albedoColorArr[3] = {albedoColor.x(), albedoColor.y(), albedoColor.z()};

        //     if (ImGui::ColorEdit3("Albedo Color", albedoColorArr))
        //     {
        //         material->setAlbedoColor(Eigen::Vector3f(albedoColorArr[0], albedoColorArr[1], albedoColorArr[2]));
        //     }

        //     // -------------------------------------------------------------
        //     // 编辑 Roughness Factor
        //     // -------------------------------------------------------------
        //     float roughnessFactor = material->getRoughnessFactor();
        //     if (ImGui::SliderFloat("Roughness Factor", &roughnessFactor, 0.0f, 1.0f))
        //     {
        //         material->setRoughnessFactor(roughnessFactor);
        //     }

        //     // -------------------------------------------------------------
        //     // 编辑 Metallic Factor
        //     // -------------------------------------------------------------
        //     float metallicFactor = material->getMetallicFactor();
        //     if (ImGui::SliderFloat("Metallic Factor", &metallicFactor, 0.0f, 1.0f))
        //     {
        //         material->setMetallicFactor(metallicFactor);
        //     }
        // } else {
        //     ImGui::Text("No material assigned.");
        // }
    }
    else
    {
        ImGui::Text("No object selected.");
        ImGui::Text("Click on an object to select it.");
    }

    ImGui::End();

    // --- 光源属性面板 ---
    ImGui::Begin("Light Properties"); 
    if (uiSceneData.pointLight) { 

        ImGui::Text("Light Parameters");

        // 光源位置 (Vector3f)
        float lightPos[3] = {uiSceneData.pointLight->position.x(), uiSceneData.pointLight->position.y(), uiSceneData.pointLight->position.z()};
        if (ImGui::SliderFloat3("Position", lightPos, -10.0f, 10.0f)) { 
            uiSceneData.pointLight->position = Eigen::Vector3f(lightPos[0], lightPos[1], lightPos[2]);
        }

        // 光源强度 (float)
        if (ImGui::SliderFloat("Intensity", &uiSceneData.pointLight->intensity, 0.0f, 20.0f)) { 
            // 强度改变，数据已更新
        }

        // 光源颜色 (Vector3f，作为 RGB 颜色选择器)
        float lightColor[3] = {uiSceneData.pointLight->color.x(), uiSceneData.pointLight->color.y(), uiSceneData.pointLight->color.z()};
        if (ImGui::ColorEdit3("Color", lightColor)) {
            uiSceneData.pointLight->color = Eigen::Vector3f(lightColor[0], lightColor[1], lightColor[2]);
        }
    } else {
        ImGui::Text("No light in scene.");
    }
    ImGui::End();

    if (showDemoWindow)
    {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }
}