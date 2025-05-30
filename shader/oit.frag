#version 450 core

in VS_OUT {
    vec2 texCoords;
} fs_in;

layout(location = 0) out vec4 accum;   // 预乘颜色 + alpha的累计
layout(location = 1) out float reveal; // revealage

uniform sampler2D albedoMap;
uniform bool hasAlbedoMap;


void main() {
    // 线性空间颜色，未做 gamma 校正，假设已是线性
    vec4 color = texture(albedoMap, fs_in.texCoords);
    //test alpha
    color.a = 0.9;

    // 计算权重函数
    // color.a 是 alpha，color.rgb 最大值控制颜色权重，gl_FragCoord.z 深度归一化到 [0,1]
    float weight = max(min(1.0, max(max(color.r, color.g), color.b) * color.a), color.a) *
                   clamp(0.03 / (1e-5 + pow(gl_FragCoord.z / 200.0, 4.0)), 1e-2, 3e3);

    // float weight = exp(-gl_FragCoord.z * 10.0);



    // 预乘 alpha 乘权重
    accum = vec4(color.rgb * color.a, color.a) * weight;
    // accum = color;

    // revealage 代表“不透明度遮盖程度”
    // reveal = color.a;

    // reveal = (1.0 - color.a) * weight;
    // reveal = 1;
    reveal = weight;
}
