// UiSystem.cpp
#include "UiSystem.h"
#include <iostream> // for debugging output

// 构造函数
UiSystem::UiSystem(SDL_Window *window, SDL_GLContext glContext)
{
    // 1. 初始化 ImGui 上下文
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    // 2. 配置 ImGui 标志
    // 假设你已经成功更新了 ImGui 库，并且你的 imgui.h 包含了这些定义。
    // 如果仍然编译报错，请注释掉这些行。
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // 启用键盘导航
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // 启用手柄导航
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // 启用 Docking
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;     // 启用多视口/浮动窗口

    // 3. 设置 ImGui 样式
    ImGui::StyleColorsDark(); // 黑暗主题

    // 4. 当启用多视口时，调整样式以更好地融入系统窗口
    // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    //     ImGuiStyle& style = ImGui::GetStyle();
    //     style.WindowRounding = 0.0f;
    //     style.Colors[ImGuiCol_WindowBg].w = 1.0f; // 确保背景不透明
    // }

    // 5. 初始化 ImGui SDL2 后端
    ImGui_ImplSDL2_InitForOpenGL(window, glContext);

    // 6. 初始化 ImGui OpenGL3 后端 (根据你的 OpenGL 版本选择，如 #version 460)
    ImGui_ImplOpenGL3_Init("#version 460"); // 请确保这个版本号与你的着色器兼容
}

// 析构函数
UiSystem::~UiSystem()
{
    // 清理 ImGui 后端和上下文
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

// 每一帧开始时调用，准备 ImGui 渲染
void UiSystem::NewFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

// 渲染 ImGui 绘制数据
void UiSystem::Render()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // 处理 ImGui 多视口（如果启用）
    // ImGuiIO& io = ImGui::GetIO();
    // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    //     SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
    //     SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
    //     ImGui::UpdatePlatformWindows();
    //     ImGui::RenderPlatformWindowsDefault();
    //     SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    // }
}

// 将 SDL 事件转发给 ImGui
void UiSystem::ProcessEvent(SDL_Event *event)
{
    ImGui_ImplSDL2_ProcessEvent(event);
}

// 绘制自定义 ImGui 界面
void UiSystem::DrawUI(int currentFPS)
{
    // 调试窗口
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

    // ***************************************************************
    // 新增：单选按钮组
    // ***************************************************************
    for (size_t i = 0; i < uiSceneData.sceneData.size(); ++i)
    { // 从索引 1 开始
        // 获取名称和值
        const auto &item = uiSceneData.sceneData[i];
        const std::string &name = std::get<0>(item);
        int value = std::get<1>(item);

        if (name == "")
        {
            ImGui::NewLine();
            continue;
        }

        ImGui::PushID(static_cast<int>(i)); // 使用索引作为唯一ID
        if (ImGui::RadioButton(name.c_str(), &selectedRenderMode, value))
        {
            // selectedRenderMode 已经被 ImGui::RadioButton 更新为 value
            std::cout << "选择了模式: " << name << " (值 " << value << ")" << std::endl;
            if (uiSceneData.onRenderModeChanged)
            {
                uiSceneData.onRenderModeChanged(selectedRenderMode);
            }
        }
        ImGui::PopID();
        ImGui::SameLine(); // 在同一行显示下一个按钮
    }
    ImGui::NewLine(); // 换行

    // ***************************************************************

    // ***************************************************************
    // 新增：滑块
    // ***************************************************************
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

    // ***************************************************************

    ImGui::End();

    // 如果 demo 窗口被勾选，则显示它
    if (showDemoWindow)
    {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }
}