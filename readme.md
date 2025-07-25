# RST 渲染引擎 / RST Rendering Engine

RST 是一个功能丰富的延迟渲染引擎，使用 C++ 和 OpenGL 构建。项目旨在探索和实现现代渲染技术，其大部分核心代码是在与 AI 工具（ChatGPT 和 Gemini）的协作下完成的，充分展示了人机协作在现代软件开发中的高效与潜力。

RST is a feature-rich deferred rendering engine built with C++ and OpenGL. The project aims to explore and implement modern rendering techniques. A significant portion of its core code was developed in collaboration with AI tools (ChatGPT and Gemini), showcasing the efficiency and potential of human-AI collaboration in modern software development.

---

## ✨ 核心特性 / Core Features

*   **图形 API / Graphics API:** **OpenGL 4.5**

*   **渲染架构 / Rendering Architecture:**
    *   **延迟渲染 (Deferred Shading):** 高效处理多光源场景，将几何渲染与光照计算分离。 / Efficiently handles scenes with multiple light sources by separating geometry rendering from lighting calculations.
    *   **基于物理的渲染 (PBR):** 支持金属度 (Metallic)、粗糙度 (Roughness) 和环境光遮蔽 (AO) 工作流。 / Supports the Metallic, Roughness, and Ambient Occlusion (AO) workflow.
    *   **模块化渲染管线 (Modular Render Pipeline):** 渲染流程被划分为一系列独立的渲染通道 (Pass)，易于管理和扩展。 / The rendering process is divided into a series of independent passes, making it easy to manage and extend.

*   **高级视觉效果 / Advanced Visual Effects:**
    *   **阴影 (Shadows):** 基于 Shadow Map 的动态阴影。 / Dynamic shadows based on Shadow Mapping.
    *   **光照 (Lighting):**
        *   **基于图像的照明 (Image-Based Lighting, IBL):** 用于实现真实的环境光照和反射。 / For realistic environment lighting and reflections.
        *   **体积光 (God Rays):** 模拟光线穿过介质的效果。 / Simulates the effect of light scattering through a medium.
    *   **后期处理 / Post-Processing:**
        *   **屏幕空间反射 (Screen Space Reflection, SSR):** 实现光滑表面的实时反射。 / Achieves real-time reflections on smooth surfaces.
        *   **屏幕空间环境光遮蔽 (Screen Space Ambient Occlusion, SSAO):** 增强场景的深度感。 / Enhances the depth and detail of the scene.
        *   **景深 (Depth of Field):** 模拟相机对焦效果。 / Simulates camera focus effects.
        *   **泛光 (Bloom):** 增强高光区域的视觉效果。 / Enhances the visual effect of bright areas.
    *   **透明物体 (Transparency):** 支持顺序无关的透明度渲染 (OIT)。 / Supports Order-Independent Transparency (OIT).

*   **核心架构 / Core Architecture:**
    *   **实体组件系统 (Entity-Component-System, ECS):** 灵活的、数据驱动的架构。 / A flexible, data-driven architecture.
    *   **命令模式 (Command Pattern):** 解耦输入处理和游戏逻辑。 / Decouples input handling from game logic.
    *   **资源管理 (Resource Management):** 集中的管理器负责纹理、模型、着色器和材质的生命周期。 / Centralized managers handle the lifecycle of textures, models, shaders, and materials.

*   **物理 / Physics:** 集成 **Jolt Physics** 引擎，支持刚体动力学。 / Integrated with the **Jolt Physics** engine to support rigid body dynamics.

*   **用户界面 / User Interface:**
    *   **ImGui:** 用于强大的调试UI。 / For powerful debugging UIs.
    *   **RmlUi:** 用于创建高性能、数据驱动的游戏内UI。 / For creating high-performance, data-driven in-game UIs.

*   **模型与动画 / Models and Animation:**
    *   通过 **Assimp** 库支持多种模型格式导入。 / Supports various model formats via the **Assimp** library.
    *   支持骨骼动画。 / Supports skeletal animation.

---

## 📸 效果展示 / Showcase

| G-Buffer Albedo | G-Buffer Normals | G-Buffer Position |
| :---: | :---: | :---: |
| ![Albedo](doc/albedo.png) | ![Normal](doc/normal.png) | ![Position](doc/position.png) |
| **Shadow Map** | **Light Pass** | **IBL Pass** |
| ![Depth](doc/depth.png) | ![Light](doc/light.png) | ![IBL](doc/iblPass.png) |
| **SSR** | **God Rays** | **Final Composite** |
| ![SSR](doc/ssr.png) | ![God Ray](doc/god_ray.png) | ![Combined](doc/combined.png) |

---

## 🏗️ 架构设计 / Architecture

### 实体组件系统 (ECS) / Entity-Component-System (ECS)
引擎采用 **实体-组件-系统 (ECS)** 架构。这种模式将数据（组件）与逻辑（系统）分离，通过实体进行关联，提供了高度的灵活性和可扩展性。

The engine uses an **Entity-Component-System (ECS)** architecture. This pattern separates data (Components) from logic (Systems) and links them via Entities, providing high flexibility and scalability.

### 渲染管线 / Render Pipeline
渲染流程被精心划分为一系列独立的 **Pass**（通道）。`Scene::run()` 方法负责按顺序调度各个渲染Pass，逐步构建出最终的图像。这种设计使得添加、移除或重排渲染效果变得非常简单。

The rendering process is carefully divided into a series of independent **Passes**. The `Scene::run()` method orchestrates these passes in sequence to progressively build the final image. This design makes it very simple to add, remove, or reorder rendering effects.

#### 渲染流程详解 / Render Pipeline Details

1.  **天空盒 (Sky Pass):** 渲染天空盒，为场景提供环境背景和IBL基础。 / Renders the skybox, providing an environmental background and the basis for IBL.
2.  **阴影 (Shadow Pass):** 从光源视角渲染场景，生成阴影贴图。 / Renders the scene from the light's perspective to generate a shadow map.
3.  **G-Buffer Pass:** 延迟渲染的核心步骤。将不透明物体的几何信息（位置、法线、反照率、PBR属性等）渲染到G-Buffer中。 / The core step of deferred rendering. Renders the geometric information of opaque objects (position, normals, albedo, PBR properties, etc.) into the G-Buffer.
4.  **SSAO Pass:** 利用G-Buffer中的信息计算环境光遮蔽，增强细节。/ Uses information from the G-Buffer to calculate ambient occlusion, enhancing details.
5.  **光照 (Light Pass):** 利用G-Buffer的数据，计算直接光照，并结合阴影贴图。 / Calculates direct lighting using data from the G-Buffer and the shadow map.
6.  **透明 (OIT Pass):** 使用顺序无关的透明度技术渲染透明物体。 / Renders transparent objects using Order-Independent Transparency techniques.
7.  **IBL Pass:** 基于环境贴图计算间接光照和反射。 / Calculates indirect lighting and reflections based on environment maps.
8.  **SSR Pass:** 在屏幕空间中计算局部反射。 / Computes local reflections in screen space.
9.  **组合 (Combined Pass):** 将直接光照、IBL、SSR、体积光、透明物体、天空盒等所有结果混合，生成最终场景图像。 / Blends all results (direct light, IBL, SSR, god rays, transparency, skybox, etc.) to generate the final scene image.
10. **后期处理 (Post-processing):**
    *   **亮度提取 (Brightness Mask):** 为Bloom效果提取高亮区域。 / Extracts bright areas for the bloom effect.
    *   **模糊 (Blur Passes):** 高斯模糊，用于Bloom和景深。 / Applies Gaussian blur for bloom and depth of field.
    *   **景深 (Depth of Field):** 模拟相机景深。 / Simulates the camera's depth of field effect.
    *   **最终合成 (Post Pass):** 应用色调映射、Gamma校正等最终调整。 / Applies final adjustments like tone mapping and gamma correction.
11. **屏幕输出 (Screen Pass):** 将最终渲染结果绘制到屏幕上。 / Draws the final rendered image to the screen.

---

## 🛠️ 依赖库 / Dependencies

本项目依赖于以下优秀的第三方库：

This project relies on the following excellent third-party libraries:

*   **SDL2:** 跨平台窗口和输入管理。 / Cross-platform window and input management.
*   **Glad:** OpenGL 函数加载器。 / OpenGL function loader.
*   **Assimp:** 模型加载库。 / Model loading library.
*   **GLI:** 纹理加载和处理。 / Texture loading and processing.
*   **ImGui:** 调试UI库。 / Debug UI library.
*   **RmlUi:** 高性能游戏UI库。 / High-performance game UI library.
*   **Jolt Physics:** 物理引擎。 / Physics engine.
*   **Eigen:** C++ 模板数学库。 / C++ template library for linear algebra.
*   **LodePNG:** PNG 图像读写。 / PNG image reading and writing.
*   **FreeType:** 字体渲染。 / Font rendering.

---

## 🚀 快速开始 / Quick Start

### 构建环境 / Prerequisites

*   C++17 编译器 (MSVC, GCC, Clang) / C++17 Compiler (MSVC, GCC, Clang).
*   CMake 3.10+

### 构建步骤 / Build Steps

```bash
# 1. 克隆仓库 / Clone the repository
git clone <your-repo-url>
cd <repo-name>

# 2. 创建构建目录 / Create a build directory
mkdir build
cd build

# 3. 运行 CMake 生成构建系统 / Run CMake to generate the build system
cmake ..

# 4. 编译项目 / Build the project
cmake --build .
```

可执行文件将生成在 `build/Debug` 或 `build/Release` 目录下。

The executable will be located in the `build/Debug` or `build/Release` directory.

---

## ⌨️ 操作控制 / Controls

*   **W, A, S, D:** 向前、左、后、右移动相机 / Move camera forward, left, backward, right.
*   **Q, E:** 上下移动相机 / Move camera up and down.
*   **鼠标移动 / Mouse Movement:** 旋转相机视角 / Rotate the camera view.
*   **鼠标滚轮 / Mouse Wheel:** 调整相机移动速度 / Adjust camera movement speed.