#version 330 core
layout(location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    // 保留旋转，去除平移部分的 view 矩阵已在 CPU 端处理
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww; // 保持 w 分量一致，防止远裁剪（z = w）
    TexCoords = aPos;
}
