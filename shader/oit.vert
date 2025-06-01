#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
    vec2 texCoords;
    vec3 worldPos;
} vs_out;


uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    vs_out.texCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vs_out.worldPos = vec3(model * vec4(aPos, 1.0));
}