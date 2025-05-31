#version 450 core

in VS_OUT {
    vec2 texCoords;
} fs_in;

layout(location = 0) out vec4 accum;   // 预乘颜色+alpha累计
layout(location = 1) out float reveal; // revealage
// layout(location = 1) out vec4 reveal; // revealage

uniform sampler2D albedoMap;
uniform bool hasAlbedoMap;

// 摄像机近平面和远平面距离（需CPU端传入）
// uniform float znear;
// uniform float zfar;

void main() {
    // 采样颜色（假设线性空间），默认alpha=1（不透明）
    vec4 color = texture(albedoMap, fs_in.texCoords);
    
    // 测试用硬编码透明度，可自行替换为color.a
    color.a = 0.5;

    // 将非线性深度gl_FragCoord.z映射到线性深度范围[znear, zfar]
    // float weight = max(
    //     min(1.0, max(max(color.r, color.g), color.b) * color.a),
    //     color.a
    // ) * clamp(0.03 / (1e-5 + pow(gl_FragCoord.z / 200, 4.0)), 1e-2, 3e3);

    float depthFactor = clamp(0.03 / (1e-5 + pow(gl_FragCoord.z / 200.0, 4.0)), 1e-2, 3e3);
    float base = max(max(color.r, color.g), color.b) * color.a;
    float weight = max(base, color.a) * depthFactor;

    // blend func: GL_ONE, GL_ONE
    // switch to pre-multiplied alpha and weight
    // accum = vec4(color.rgb * color.a, color.a) * weight * 0.0005;
    accum = vec4(color.rgb * color.a, color.a) * weight;

    // blend func: GL_ZERO, GL_ONE_MINUS_SRC_ALPHA
    reveal = color.a;
    // reveal = 0.5;
    
}