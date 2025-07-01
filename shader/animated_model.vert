#version 330 core // 保持你原始的 330 core 版本

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

// 新增：骨骼ID和权重属性
layout (location = 5) in ivec4 aBoneIDs;
layout (location = 6) in vec4 aBoneWeights;

out VS_OUT {
    vec3 fragPos;     // 世界空间中的片段位置
    vec3 normal;      // 世界空间中的法线
    vec2 texCoords;   // 纹理坐标 (注意：这里是小写 'texCoords')
    mat3 TBN;         // 切线空间到世界空间的转换矩阵
    vec4 clipPos;     // 裁剪空间位置
    float clipW;      // 裁剪空间W分量，用于透视除法后的深度重构等
} vs_out;


uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;             // Model自身的全局变换矩阵
uniform mat4 meshLocalTransform; // Mesh自身的局部变换矩阵

// 用于蒙皮的骨骼变换矩阵数组 (从Animator传入)
uniform mat4 finalBoneMatrices[100]; // 对应 MAX_BONES

// 一个布尔值，告诉Shader当前模型是否需要动画处理
uniform bool isAnimated;

void main()
{
    // 计算最终的模型-网格变换矩阵
    mat4 finalModelMatrix = model * meshLocalTransform; 

    // 声明动画/非动画状态下的顶点属性
    vec4 processedPos = vec4(aPos, 1.0);
    vec3 processedNormal = aNormal;
    vec3 processedTangent = aTangent;
    vec3 processedBitangent = aBitangent;

    if (isAnimated)
    {
        mat4 boneTransform = mat4(0.0); // 初始化为零矩阵
        
        // 累加所有影响该顶点的骨骼的变换
        for(int i = 0; i < 4; ++i) // 假设每个顶点最多受4根骨骼影响
        {
            if (aBoneWeights[i] > 0.0) // 仅对有权重的骨骼进行处理
            {
                // 将每个骨骼的变换矩阵按权重累加
                boneTransform += finalBoneMatrices[aBoneIDs[i]] * aBoneWeights[i];
            }
        }
        
        // 将顶点位置应用骨骼变换 (蒙皮)
        // 从模型空间（绑定姿态）转换到动画姿态的模型空间
        processedPos = boneTransform * vec4(aPos, 1.0);
        
        // 法线、切线、副切线应用骨骼变换 (只应用旋转和均匀缩放部分)
        // 假设骨骼变换不含非均匀缩放，直接使用3x3部分来变换这些向量
        processedNormal = mat3(boneTransform) * aNormal;
        processedTangent = mat3(boneTransform) * aTangent;
        processedBitangent = mat3(boneTransform) * aBitangent;
    }
    // else 分支不需要额外处理，因为 processedPos/Normal/Tangent/Bitangent 已经默认初始化为 aPos/aNormal/aTangent/aBitangent

    // 1. 计算世界空间位置
    vec4 worldPos = finalModelMatrix * processedPos;
    vs_out.fragPos = worldPos.xyz;

    // 2. 计算法线矩阵 (Normal Matrix)
    // 这是正确的法线、切线、副切线变换方式，适用于包含非均匀缩放的模型
    mat3 normalMatrix = mat3(transpose(inverse(finalModelMatrix)));

    // 3. 变换法线、切线、副切线到世界空间，并进行归一化
    // 注意：这里的输出变量名与 VS_OUT 结构体定义保持一致（小写）
    vec3 _normal = normalize(normalMatrix * processedNormal);
    vec3 _tangent = normalize(normalMatrix * processedTangent);
    // 确保副切线与法线和切线正交，重新计算，而不是直接变换原始的aBitangent
    vec3 _bitangent = normalize(cross(_normal, _tangent)); 

    // 4. 构建 TBN 矩阵 (Tangent-Bitangent-Normal)
    // TBN 矩阵的列是切线、副切线、法线在世界空间中的向量
    vs_out.TBN = mat3(_tangent, _bitangent, _normal);
    
    // 5. 纹理坐标直接传递 (注意：这里的输出变量名与 VS_OUT 结构体定义保持一致)
    vs_out.texCoords = aTexCoords;

    // 6. 计算最终的裁剪空间位置和 clipW
    vec4 eyePos = view * worldPos;
    vs_out.clipPos = projection * eyePos;
    vs_out.clipW = vs_out.clipPos.w; 

    // 设置 gl_Position
    gl_Position = vs_out.clipPos;
}