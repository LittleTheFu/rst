#version 330 core
in vec4 fragColor; // 从顶点着色器接收的颜色
out vec4 color;     // 最终输出到帧缓冲的颜色

void main()
{
    color = fragColor; // 直接使用传入的颜色
}