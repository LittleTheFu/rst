#version 450 core

layout (location = 0) in vec3 aPos;
out vec3 FragPosWorld;


uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;       // 模型的模型矩阵

void main() {
    mat4 lightSpaceMatrix = projection * view;
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
    FragPosWorld = vec3(model * vec4(aPos, 1.0));  // world-space position
}
