项目简介：基于 OpenGL 的延迟渲染引擎
本项目是一个基于 OpenGL 开发的延迟渲染引擎，致力于呈现高质量的图形渲染。值得一提的是，其大部分核心代码是在与 ChatGPT 和 Gemini 等 AI 工具的对话协作下完成的，这充分展现了人机协作在现代软件开发中的高效与潜力。项目目前仍在积极的迭代与完善中。

核心特性与技术栈

延迟渲染 (Deferred Shading)：项目采用了延迟渲染架构，巧妙地将几何信息的渲染与光照计算分离。这种设计尤其适合处理包含大量光源的复杂场景，能够显著提升渲染效率。

模块化渲染管线：整个渲染流程被精心划分为一系列独立的“Pass”（通道），每个Pass专注于完成特定的渲染任务，这种模块化设计极大地提高了项目的可管理性和可扩展性。

丰富的高级视觉效果：引擎实现了多种先进的图形效果，包括模拟相机镜头的景深（Depth of Field）、光线穿透介质的体积光（God Ray）、基于屏幕信息的屏幕空间反射（Screen Space Reflection, SSR）、模拟环境光遮蔽的AO（Ambient Occlusion），以及**基于图像的光照（Image-Based Lighting, IBL）**等，致力于提供更具真实感的视觉体验。

AI 辅助开发实践：本项目是利用 AI 进行代码生成、问题排查和架构设计的成功案例，展示了 AI 在辅助软件开发方面的强大能力。

----------------------------------------------

渲染流程详解与Pass概览
项目的渲染核心在于 Scene::run() 方法，它像一个精密的指挥家，按序调度各个渲染Pass，逐步构建最终的图像。

场景数据更新 (updateScene())
每帧开始时，此函数会更新场景中的动态元素，例如演示中光源位置的动画变化，以及调试用的物体同步移动。

----------------------------------------------

Sky Pass (天空通道)

职责：渲染场景的背景天空盒，为后续光照提供环境基础。

结果：仅包含天空和环境的图像。

----------------------------------------------

Shadow Pass (阴影通道)

职责：从光源视角渲染场景深度，生成一张阴影贴图，用于标记哪些区域被遮挡而处于阴影中
。
结果：记录阴影信息的“阴影贴图”。

----------------------------------------------

G-Buffer Pass (几何缓冲区通道)

职责：延迟渲染的核心阶段。将所有不透明物体的几何信息（如世界空间位置、法线、反照率、粗糙度、金属度、AO 和深度）渲染并存储到多张纹理中，统称为 G-Buffer。

结果：包含物体位置、法线、反照率、粗糙度、金属度、环境光遮蔽 (AO) 和深度信息的 G-Buffer 纹理集。

----------------------------------------------
OIT Pass (顺序无关透明度通道)

职责：高效处理透明物体的渲染，确保即便渲染顺序不定，透明度也能正确混合叠加。

结果：处理后的透明物体视觉效果。

----------------------------------------------

Light Pass (光照通道)

职责：利用 G-Buffer 中的几何数据，对每个像素执行直接光照计算，并结合阴影贴图确定光照贡献。

结果：只包含直接光照效果的图像。

----------------------------------------------

Brightness Mask Pass (亮度遮罩通道)

职责：从光照结果中提取亮度超过阈值的区域，生成一个亮度遮罩，这是实现泛光（Bloom）效果的基础。

结果：突出显示最亮区域的“遮罩图像”。

----------------------------------------------

God Ray Pass (体积光通道)

职责：模拟光线穿透介质形成的可见光束效果，如丁达尔效应。

结果：仅包含体积光效果的图像。

----------------------------------------------

IBL Pass (基于图像的光照通道)

职责：利用环境贴图（辐照度图、预过滤图）和 BRDF 查找表，为场景物体添加环境光照和反射效果。

结果：包含环境光照和反射效果的图像。

----------------------------------------------

SSR Pass (屏幕空间反射通道)

职责：在屏幕空间内高效计算反射效果，常用于光滑表面的局部反射。

结果：只包含屏幕空间反射效果的图像。

----------------------------------------------

Combined Pass (组合通道)

职责：将上述所有独立的渲染结果（直接光照、IBL、SSR、体积光、透明物体、天空盒等）进行最终混合与叠加，生成一张完整的场景图像。

结果：整合了所有光照和特殊效果的完整图像。

----------------------------------------------

Blur Horizontal Pass & Blur Vertical Pass (水平/垂直模糊通道)

职责：对图像进行水平和垂直方向的模糊处理，常作为景深、泛光等后期效果的中间步骤。

结果：模糊处理后的图像。

----------------------------------------------

Depth of Field Pass (景深通道)

职责：模拟真实相机镜头的景深效果，使焦点区域清晰，而焦点前后区域模糊，增强图像的艺术感。

结果：应用景深效果的图像。

----------------------------------------------

Post Pass (后期处理通道)

职责：对最终图像进行一系列后期处理，如色彩校正、锐化等，进一步提升视觉质量。

结果：最终增强处理后的图像。

----------------------------------------------

Screen Pass (屏幕输出通道)

职责：将最终的渲染图像绘制到全屏四边形上，呈现在用户的显示器上。

结果：用户在显示器上看到的最终图像。

-------------------------------------------------

依赖与可扩展性
本项目集成了 SDL、Assimp、lodepng、GLI、ImGui 和 Eigen 等知名的第三方库，这些库为窗口管理、模型导入、图像处理、UI 调试和数学运算提供了坚实的基础。模块化的设计和清晰的 Pass 结构使得项目具有良好的可扩展性，未来可以方便地添加更多高级渲染技术和效果。

-----------------------------------------------------

----------------------------------------------
暂时的简介：这个是用opengl写的一个延迟渲染引擎，大部分是通过和ai对话完成的，用的是免费版的chatgpt和gemini。还在进行中

QUICK START GUIDE:

mkdir build

cd build

cmake ..

cmake --build .


you'll find the executable in the build directory.

------------------------------------------------------
# My Project Readme

## Images

Here are some visual assets from my project:

### Albedo Map
![Albedo](doc/albedo.png)

### Ambient Occlusion (AO)
![AO](doc/ao.png)

### Horizontal Blur
![Blur Horizontal](doc/blur_horizontal.png)

### Vertical Blur
![Blur Vertical](doc/blur_vertical.png)

### Brightness Mask
![Brightness Mask](doc/brightness_mask.png)

### Combined Image
![Combined](doc/combined.png)

### Depth Map
![Depth](doc/depth.png)

### Depth of Field
![Depth of Field](doc/depth_of_field.png)

### God Ray Effect
![God Ray](doc/god_ray.png)

### Light Map
![Light](doc/light.png)

### Metallic Map
![Metallic](doc/metallic.png)

### Normal Map
![Normal](doc/normal.png)

### Position Map
![Position](doc/position.png)

### Post-processing
![Post](doc/post.png)

### Roughness Map
![Roughness](doc/roughness.png)

### Skybox
![Skybox](doc/skybox.png)

### Screen Space Reflections (SSR)
![SSR](doc/ssr.png)

----------------------------------------------------

## Rendering Process
1. Sky Pass

    Result: A picture showing only the sky and environment.

2. Shadow Pass

    Result: A "shadow map" that records where shadows fall.

3. G-Buffer Pass (Geometry Buffer Pass)

    Result: A collection of "X-ray plates" containing all the necessary information to render objects, but without their final color.

    Position Plate: Records the exact 3D spatial position of each point.

    Normal Plate: Records the surface orientation of each point (how light reflects differently based on direction).

    Albedo Plate: Records the object's base color.

    Roughness Plate: Records how smooth or rough the object's surface is.

    Metallic Plate: Records whether the object is a metallic material.

    Ambient Occlusion Plate (AO): Records how dark certain areas (like cracks or corners) appear due to being obstructed from ambient light.

    Depth Plate: Records how far each point is from the camera.

4. OIT Pass (Order-Independent Transparency)

    Result: Processed transparent object visuals, ready to be combined with other parts of the scene later.

5. Light Pass

    Result: A picture showing only the direct lighting effects.

6. Brightness Mask Pass

    Result: A "mask image" that highlights only the brightest areas.

7. God Ray Pass

    Result: A picture showing only the volumetric light effect.

8. IBL Pass (Image-Based Lighting)

    Result: A picture including environmental lighting and reflection effects.

9. SSR Pass (Screen Space Reflection)

    Result: A picture showing only the screen-space reflection effects.

10. Combined Pass

    Result: A complete image incorporating all lighting and special effects.

11. Blur Horizontal Pass & Blur Vertical Pass

    Result: A blurred image.

12. Depth of Field Pass

    Result: An image with a depth-of-field effect.

13. Post Pass

    Result: The final, enhanced image.

14. Screen Pass

    Result: The image you see on your display.

## How It Works
The entire process operates like an assembly line, with each step focusing on a specific task. Ultimately, all these effects are layered together to create the realistic and detailed computer graphics you see.
