#version 450 core

in vec3 FragPosWorld;

out float FragDepth;

uniform vec3 lightPos;
uniform float farClip;

void main() {
    float distance = length(FragPosWorld - lightPos);
    // 归一化写入 [0,1]
    FragDepth = distance / farClip;//这里不是写入深度buffer
    gl_FragDepth = distance / farClip;
}
