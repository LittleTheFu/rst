#version 330 core
layout (location = 0) in vec3 aPos;   // 顶点位置，来自 JoltDebugRenderer::mTriangleVBO 或 mLineVBO
layout (location = 1) in vec4 aColor; // 顶点颜色，来自 JoltDebugRenderer::mTriangleColorVBO 或 mLineColorVBO

out vec4 fragColor; // 传递给片段着色器的颜色

uniform mat4 model;      // 模型矩阵 (对于调试几何体通常是单位矩阵)
uniform mat4 view;       // 视图矩阵 (来自你的相机)
uniform mat4 projection; // 投影矩阵 (来自你的相机)

void main()
{
    fragColor = aColor; // 将顶点颜色直接传递给片段着色器
    gl_Position = projection * view * model * vec4(aPos, 1.0); // 计算最终的裁剪空间位置
}