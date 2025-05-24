#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 texCoords;
    mat3 TBN;
    vec4 clipPos;
    float clipW; // 新增输出
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    vec4 eyePos = view * worldPos;
    vec4 clipPos = projection * eyePos;
    vs_out.fragPos = vec3(worldPos);
    vs_out.normal = mat3(transpose(inverse(model))) * aNormal;
    vs_out.texCoords = aTexCoords;
    vec3 T_world = normalize(mat3(transpose(inverse(model))) * aTangent);
    vec3 N_world = normalize(mat3(transpose(inverse(model))) * aNormal);
    vec3 B_world = normalize(cross(N_world, T_world));
    vs_out.TBN = mat3(T_world, B_world, N_world);
    vs_out.clipPos = clipPos;
    vs_out.clipW = clipPos.w; // 输出 clipW
    gl_Position = clipPos;
}