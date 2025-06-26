#include "UiSystem.h"
#include <iostream>
#include <mesh.h>

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

    // 获取当前被选中的 Mesh (用于高亮显示)
    Mesh *currentSelectedMesh = nullptr;
    if (onGetSelectedMesh)
    { // onGetSelectedMesh 是 UiSystem 的成员
        currentSelectedMesh = onGetSelectedMesh();
    }

    // 遍历所有 Mesh 并在列表中显示
    if (!uiSceneData.allMeshes.empty())
    {
        for (size_t i = 0; i < uiSceneData.allMeshes.size(); ++i)
        {
            Mesh *mesh = uiSceneData.allMeshes[i];
            if (!mesh)
                continue; // 安全检查，防止空指针

            // 设置选中状态：如果当前 Mesh 是被选中的，则 ImGui::Selectable 会高亮显示
            bool isSelected = (mesh == currentSelectedMesh);

            // 为每个 Selectable 加上唯一的 ID，防止列表项名称重复时 ImGui 混淆
            // (mesh->getName().c_str() + i) 是一种简单的生成唯一 ID 的方式
            // 或者更推荐使用 ImGui::PushID(mesh) 和 ImGui::PopID()
            ImGui::PushID(mesh); // 使用 Mesh* 地址作为 ID
            if (ImGui::Selectable(mesh->getName().c_str(), isSelected))
            {
                // 如果用户点击了这个列表项，并且我们有设置选中 Mesh 的回调
                if (uiSceneData.onMeshSelectedFromUI)
                {                                           // 通过 uiSceneData 访问回调
                    uiSceneData.onMeshSelectedFromUI(mesh); // 通知 Scene 选中了这个 Mesh
                }
            }
            ImGui::PopID(); // 匹配 PushID
        }
    }
    else
    {
        ImGui::Text("No meshes in scene.");
    }

    ImGui::End(); // End Scene Outliner Window

    // --- 选中物体属性窗口 ---
    ImGui::Begin("Selected Object Properties");

    Mesh *selectedMesh = onGetSelectedMesh();
    if (selectedMesh)
    { // 检查指针是否为空
        // 显示名称 (Text 控件，不编辑)
        ImGui::Text("Selected Object Name: %s", selectedMesh->getName().c_str());

        ImGui::Separator();

        // -------------------------------------------------------------------
        // 编辑位置 (Position)
        // -------------------------------------------------------------------
        // 将 Eigen::Vector3f 转换为 float[3] 数组供 ImGui::InputFloat3 使用
        Eigen::Vector3f currentPos = selectedMesh->getPosition();
        float pos[3] = {currentPos.x(), currentPos.y(), currentPos.z()};

        // ImGui::InputFloat3 会返回 true 如果值被用户修改了
        if (ImGui::InputFloat3("Position", pos))
        {
            // 如果修改了，更新 Mesh 的位置
            selectedMesh->setPosition(Eigen::Vector3f(pos[0], pos[1], pos[2]));
            // (可选) 可以在这里打印日志或触发一个事件
            // std::cout << "Mesh " << selectedMesh->getName() << " new Position: "
            //           << pos[0] << ", " << pos[1] << ", " << pos[2] << std::endl;
        }

        // -------------------------------------------------------------------
        // 编辑旋转 (Rotation) - 使用欧拉角
        // -------------------------------------------------------------------
        Eigen::Quaternionf currentRotQuat = selectedMesh->getRotation();
        // 将四元数转换为欧拉角（度），供 ImGui 编辑
        // 顺序 (0,1,2) 对应 X(Roll), Y(Pitch), Z(Yaw)
        // 记住 ImGui InputFloat3 的值是弧度还是度取决于你的惯例，这里我们用度
        Eigen::Vector3f currentEuler = currentRotQuat.toRotationMatrix().eulerAngles(0, 1, 2) * 180.0f / M_PI;
        float rot[3] = {currentEuler.x(), currentEuler.y(), currentEuler.z()};

        if (ImGui::InputFloat3("Rotation (Euler)", rot))
        {
            // 如果欧拉角被修改了，将其转换回四元数并更新 Mesh
            // 注意：欧拉角到四元数的转换顺序必须和提取时一致，且要从度转回弧度
            Eigen::Quaternionf newRotQuat =
                Eigen::AngleAxisf(rot[2] * M_PI / 180.0f, Eigen::Vector3f::UnitZ()) * // Yaw
                Eigen::AngleAxisf(rot[1] * M_PI / 180.0f, Eigen::Vector3f::UnitY()) * // Pitch
                Eigen::AngleAxisf(rot[0] * M_PI / 180.0f, Eigen::Vector3f::UnitX());  // Roll

            selectedMesh->setRotation(newRotQuat);
            // (可选) 打印日志
            // std::cout << "Mesh " << selectedMesh->getName() << " new Euler: "
            //           << rot[0] << ", " << rot[1] << ", " << rot[2] << std::endl;
        }

        // -------------------------------------------------------------------
        // 编辑缩放 (Scale)
        // -------------------------------------------------------------------
        Eigen::Vector3f currentScale = selectedMesh->getScale();
        float scale[3] = {currentScale.x(), currentScale.y(), currentScale.z()};

        if (ImGui::InputFloat3("Scale", scale))
        {
            selectedMesh->setScale(Eigen::Vector3f(scale[0], scale[1], scale[2]));
            // (可选) 打印日志
            // std::cout << "Mesh " << selectedMesh->getName() << " new Scale: "
            //           << scale[0] << ", " << scale[1] << ", " << scale[2] << std::endl;
        }

        ImGui::Separator(); // 分隔线
        ImGui::Text("Material Properties:");

        // 尝试获取选中 Mesh 的材质
        std::shared_ptr<Material> material = selectedMesh->getMaterial();

        if (material)
        { // 确保 Mesh 关联了一个材质
            // 显示材质名称 (通常是只读的，用于识别)
            ImGui::Text("Material Name: %s", material->getName().c_str());

            // -------------------------------------------------------------
            // 编辑 Albedo 颜色
            // -------------------------------------------------------------
            // 获取当前的 Albedo 颜色 (Eigen::Vector3f)
            Eigen::Vector3f albedoColor = material->getAlbedoColor();
            // 将 Eigen::Vector3f 转换成 float[3] 数组供 ImGui 使用
            float albedoColorArr[3] = {albedoColor.x(), albedoColor.y(), albedoColor.z()};

            // ImGui::ColorEdit3 返回 true 如果颜色被修改
            if (ImGui::ColorEdit3("Albedo Color", albedoColorArr))
            {
                // 如果颜色被修改，将 float[3] 转换回 Eigen::Vector3f 并更新材质
                material->setAlbedoColor(Eigen::Vector3f(albedoColorArr[0], albedoColorArr[1], albedoColorArr[2]));
            }

            // -------------------------------------------------------------
            // 编辑 Roughness Factor
            // -------------------------------------------------------------
            float roughnessFactor = material->getRoughnessFactor();
            // ImGui::SliderFloat 更适合有范围 (0.0 - 1.0) 的浮点数
            if (ImGui::SliderFloat("Roughness Factor", &roughnessFactor, 0.0f, 1.0f))
            {
                material->setRoughnessFactor(roughnessFactor);
            }

            // -------------------------------------------------------------
            // 编辑 Metallic Factor
            // -------------------------------------------------------------
            float metallicFactor = material->getMetallicFactor();
            if (ImGui::SliderFloat("Metallic Factor", &metallicFactor, 0.0f, 1.0f))
            {
                material->setMetallicFactor(metallicFactor);
            }
        }
    }
    else
    {
        ImGui::Text("No object selected.");
        ImGui::Text("Click on an object to select it.");
    }

    ImGui::End();

    if (showDemoWindow)
    {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }
}
