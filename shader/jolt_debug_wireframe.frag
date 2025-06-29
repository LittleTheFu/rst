#version 330 core
// in vec4 fragColor; // 如果你想忽略传入的顶点颜色，可以注释掉这行
out vec4 color;

void main()
{
    color = vec4(0.0, 1.0, 0.0, 1.0); // 亮绿色，完全不透明
}