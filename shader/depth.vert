#version 450 core // 建议使用 450 core，因为它支持你在 VertexArray 中使用的 DSA 函数，且与原始 Shadow Map shader 一致

layout (location = 0) in vec3 aPos;

// 新增：骨骼ID和权重属性，与G-Pass Shader中的location保持一致
layout (location = 5) in ivec4 aBoneIDs;
layout (location = 6) in vec4 aBoneWeights;

// 重新添加 FragPosWorld 输出
out vec3 FragPosWorld; 

uniform mat4 lightSpaceMatrix; // 光源的视锥体变换矩阵 (Projection * View)
uniform mat4 model;            // Model自身的全局变换矩阵
uniform mat4 meshLocalTransform; // Mesh自身的局部变换矩阵

// 动画 Uniforms
uniform mat4 finalBoneMatrices[100];
uniform bool isAnimated;

void main() {
    // 计算最终的模型-网格变换矩阵
    // 这与你的G-Pass Shader中的计算方式保持一致，确保了模型在世界空间中的正确位置
    mat4 finalModelMatrix = model * meshLocalTransform; 

    // 声明动画/非动画状态下的顶点位置
    vec4 processedPos = vec4(aPos, 1.0); // 默认初始化为绑定姿态的顶点位置

    if (isAnimated) {
        mat4 boneTransform = mat4(0.0); // 初始化为零矩阵
        
        // 累加所有影响该顶点的骨骼的变换
        for(int i = 0; i < 4; ++i) { // 假设每个顶点最多受4根骨骼影响
            if (aBoneWeights[i] > 0.0) { // 仅对有权重的骨骼进行处理
                // 将每个骨骼的变换矩阵按权重累加
                // 注意：这里 BoneIDs 和 BoneWeights 的索引必须有效，且与实际的骨骼数据匹配
                boneTransform += finalBoneMatrices[aBoneIDs[i]] * aBoneWeights[i];
            }
        }
        
        // 将顶点位置应用骨骼变换 (蒙皮)
        // 从模型空间（绑定姿态）转换到动画姿态的模型空间
        processedPos = boneTransform * vec4(aPos, 1.0);
    }
    // 如果 isAnimated 为 false，processedPos 保持为原始的 aPos

    // 计算最终的裁剪空间位置，这是Shadow Map的核心
    // processedPos 已经是应用了蒙皮后的顶点位置 (模型空间)
    gl_Position = lightSpaceMatrix * finalModelMatrix * processedPos;

    // 计算世界空间位置并输出，与原始的 Shadow Map shader 保持一致
    FragPosWorld = vec3(finalModelMatrix * processedPos); // world-space position after animation
}