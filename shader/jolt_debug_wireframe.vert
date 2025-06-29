#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor; // 线框通常也需要颜色

out vec4 fragColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    fragColor = aColor; // 或使用统一颜色
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}