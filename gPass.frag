#version 450 core

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

out vec4 FragColor;

void main() {
    // 这里是最简单的片段着色器，直接输出一个固定的颜色
    FragColor = vec4(1.0, 0.0, 0.0, 1.0); // 红色
}