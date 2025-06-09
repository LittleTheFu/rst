// #version 450 core

// in vec2 TexCoords;
// out float FragColor;

// uniform sampler2D gPosition;
// uniform sampler2D gNormal;
// uniform sampler2D texNoise;

// uniform vec3 samples[64];
// uniform mat4 projection;

// const float radius = 0.5;
// const float bias = 0.025;

// const vec2 noiseScale = vec2(1280.0 / 4.0, 720.0 / 4.0); // 根据你的屏幕分辨率动态设置

// void main()
// {
//     // 获取片元的位置和法线（都是 View 空间）
//     vec3 fragPos = texture(gPosition, TexCoords).xyz;
//     vec3 normal = normalize(texture(gNormal, TexCoords).xyz);

//     // 随机向量用于打乱采样方向（扰动TBN空间）
//     vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);

//     // 构造 TBN 矩阵（以当前法线为基准）
//     vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
//     vec3 bitangent = normalize(cross(normal, tangent));
//     mat3 TBN = mat3(tangent, bitangent, normal);

//     float occlusion = 0.0;
//     for (int i = 0; i < 64; ++i)
//     {
//         // 将样本从切线空间变换到视空间
//         vec3 sampleVec = TBN * samples[i];
//         vec3 samplePos = fragPos + sampleVec * radius;

//         // 投影到裁剪空间 -> 归一化设备坐标
//         vec4 offset = projection * vec4(samplePos, 1.0);
//         offset.xyz /= offset.w;
//         offset.xyz = offset.xyz * 0.5 + 0.5; // NDC [-1,1] -> [0,1]

//         float sampleDepth = texture(gPosition, offset.xy).z;

//         // 当前 sample 是否被遮挡（z越大越远）
//         float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
//         if (sampleDepth >= samplePos.z + bias)
//             occlusion += rangeCheck;
//     }

//     occlusion = 1.0 - (occlusion / 64.0);
//     FragColor = occlusion;
// }
