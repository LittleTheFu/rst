#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
    vec2 texCoords;
    vec3 worldPos;
    mat3 TBN;
} vs_out;


uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    vs_out.texCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vs_out.worldPos = vec3(model * vec4(aPos, 1.0));

    vec3 T_world = normalize(mat3(transpose(inverse(model))) * aTangent);
    vec3 N_world = normalize(mat3(transpose(inverse(model))) * aNormal);
    vec3 B_world = normalize(cross(N_world, T_world));
    vs_out.TBN = mat3(T_world, B_world, N_world);
}