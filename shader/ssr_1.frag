#version 460 core

in vec2 vTexCoords;
out vec4 FragColor;

uniform sampler2D gNormal;    // View 空间法线 G-buffer
uniform sampler2D gDepth;     // 线性深度 G-buffer (0-1 范围)
uniform sampler2D sceneColor; // 最终场景颜色纹理 (包含所有不透明物体的颜色)
uniform sampler2D gMetallic;  // 金属度贴图 (PBR 材质属性)
uniform sampler2D gRoughness; // 粗糙度贴图 (PBR 材质属性)

uniform mat4 uProjectionMatrix; // 投影矩阵

// --- Helper Functions ---

// 反投影：根据屏幕 UV 坐标和深度重建 View 空间位置
vec3 reconstructViewPos(vec2 uv, float depth)
{
    // 将 [0, 1] 范围的 UV 坐标转换为 NDC 空间的 [-1, 1] 坐标
    // 深度值也从 [0, 1] 映射到 NDC 的 [-1, 1] (Z 轴)
    vec4 clip = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    
    // 使用投影矩阵的逆矩阵将 NDC 坐标反投影到 View 空间齐次坐标
    vec4 view = inverse(uProjectionMatrix) * clip;
    
    // 进行透视除法，得到 View 空间的三维坐标
    return view.xyz / view.w;
}

// 计算反射方向
// viewDir: 从物体表面指向摄像机的方向
// normal: 物体表面的法线
vec3 reflectDirection(vec3 viewDir, vec3 normal)
{
    // reflect 函数的第一个参数是入射向量，第二个参数是法线。
    // 如果 viewDir 是从物体表面指向摄像机，那么其反方向 -viewDir 
    // 才是从摄像机指向物体表面的“入射光线方向”。
    return reflect(-viewDir, normal);
}

// --- Main Shader Logic ---

void main()
{
    // 1. 获取当前像素的 G-buffer 数据
    float depth = texture(gDepth, vTexCoords).r;
    
    // 如果深度为 1.0，通常表示天空盒或背景，不进行 SSR。
    // 在没有环境贴图回退的情况下，这些区域将显示为黑色或原始场景色。
    if (depth >= 1.0) {
        FragColor = vec4(0.0); // 默认显示黑色
        return;
    }

    float metallic = texture(gMetallic, vTexCoords).r;
    float roughness = texture(gRoughness, vTexCoords).r;

    // 2. 根据 PBR 材质属性进行初步剔除
    // 粗糙度较高的材质反射不清晰，SSR 效果不明显，可以跳过。
    // 金属度较低（非金属）的材质通常没有强的镜面反射，也可跳过。
    // 这些阈值 (0.2, 0.4) 需要根据你的艺术风格和性能要求进行调整。
    if (metallic < 0.7 || roughness > 0.2) { 
        // 在没有环境贴图的情况下，直接返回黑色或当前场景颜色
        // 如果想混合回原始场景颜色，可以用：
        // FragColor = texture(sceneColor, vTexCoords);
        FragColor = vec4(0.0); 
        return; 
    }

    // 3. 计算 View 空间位置、法线和反射方向
    vec3 normal = normalize(texture(gNormal, vTexCoords).xyz * 2.0 - 1.0);
    vec3 viewPos = reconstructViewPos(vTexCoords, depth);
    vec3 viewDir = -normalize(-viewPos); // 从当前像素点指向摄像机的方向
    vec3 reflDir = reflectDirection(viewDir, normal);

    // 4. 屏幕空间射线步进参数
    const int maxSteps = 128;   // 增加最大步数以提高反射范围和连续性
    const float stepSize = 0.05; // 减小步长以提高命中精度
    const int binarySearchSteps = 8; // 二分查找的步数，提高命中精确度

    vec3 ray = reflDir;      // 反射射线方向
    vec3 origin = viewPos;   // 射线起点 (当前像素在 View 空间的位置)

    vec3 hitPos = origin;    // 射线当前步进到的 View 空间位置
    vec2 hitUV = vec2(0.0);  // 命中点的屏幕 UV 坐标
    bool hit = false;        // 是否命中屏幕内的几何体

    // 基础命中阈值，用于判断射线是否“触及”几何体
    float baseHitDist = 0.05; 

    for (int i = 0; i < maxSteps; ++i)
    {
        hitPos += ray * stepSize; // 沿着反射方向步进

        // 将当前步进到的 View 空间位置投影回屏幕空间，得到 UV 坐标
        vec4 proj = uProjectionMatrix * vec4(hitPos, 1.0);
        proj.xy /= proj.w; // 透视除法
        hitUV = proj.xy * 0.5 + 0.5; // NDC 坐标 [-1, 1] 转换到 UV 坐标 [0, 1]

        // 如果射线离开屏幕，则停止步进
        if (hitUV.x < 0.0 || hitUV.x > 1.0 || hitUV.y < 0.0 || hitUV.y > 1.0) {
            break; 
        }

        // 获取命中点在屏幕空间对应的深度
        float sceneDepth = texture(gDepth, hitUV).r;
        // 重建命中点在 View 空间的实际位置
        vec3 scenePos = reconstructViewPos(hitUV, sceneDepth);
        
        // 计算当前步进点与场景实际几何体之间的距离
        float dist = length(scenePos - hitPos);

        // 动态调整命中阈值：越远的物体，允许越大的距离误差
        // 0.05 是一个经验系数，用于调整距离对阈值的影响
        float currentThreshold = baseHitDist * (1.0 + abs(hitPos.z) * 0.05); 

        // 如果距离小于阈值，认为射线近似命中几何体
        if (dist < currentThreshold)
        {
            hit = true;
            
            // --- 进行二分查找来精确化命中点 ---
            vec3 refineStartPos = hitPos - ray * stepSize; // 回溯到前一步的起点
            vec3 refineEndPos = hitPos; // 当前命中点

            for (int j = 0; j < binarySearchSteps; ++j) 
            {
                vec3 midPos = (refineStartPos + refineEndPos) * 0.5; // 计算中点
                
                // 将中点投影回屏幕空间并获取深度
                vec4 midProj = uProjectionMatrix * vec4(midPos, 1.0);
                midProj.xy /= midProj.w;
                vec2 midUV = midProj.xy * 0.5 + 0.5;

                // 检查中点 UV 是否在屏幕内，避免采样越界
                if (midUV.x < 0.0 || midUV.x > 1.0 || midUV.y < 0.0 || midUV.y > 1.0) {
                    break; 
                }

                float midSceneDepth = texture(gDepth, midUV).r;
                vec3 midScenePos = reconstructViewPos(midUV, midSceneDepth);

                // 如果中点仍然命中（距离小于阈值），则缩小搜索范围到前半段
                if (length(midScenePos - midPos) < currentThreshold) {
                    refineEndPos = midPos;
                } else { // 否则，缩小搜索范围到后半段
                    refineStartPos = midPos;
                }
            }
            hitPos = refineEndPos; // 使用二分查找得到的更精确的命中点
            
            // 重新计算精确命中点对应的 UV 坐标，用于后续采样
            vec4 finalProj = uProjectionMatrix * vec4(hitPos, 1.0);
            finalProj.xy /= finalProj.w;
            hitUV = finalProj.xy * 0.5 + 0.5;
            
            break; // 找到命中点并精确化后，跳出主步进循环
        }
    }

    // 5. 根据命中结果和粗糙度输出最终颜色
    vec3 finalReflectedColor;

    if (hit)
    {
        // 如果命中，从场景颜色纹理中采样反射颜色
        // 如果 sceneColor 纹理生成了 mipmap，可以通过 roughness 采样对应模糊级别的 mipmap
        // 4.0 是一个经验系数，用于将粗糙度映射到 sceneColor 的 mipmap 级别。
        // 如果 sceneColor 没有 mipmap，请使用 texture(sceneColor, hitUV).rgb;
        vec3 ssrColor = textureLod(sceneColor, hitUV, roughness * 4.0).rgb;

        // 屏幕边缘衰减：防止反射在屏幕边缘突然消失
        // 当命中点接近屏幕边缘时，SSR 的贡献会逐渐衰减到0，避免突兀的黑边。
        float edgeFade = 1.0;
        float fadeDistance = 0.05; // 距离边缘 0.05 的 UV 范围开始衰减
        edgeFade *= smoothstep(0.0, fadeDistance, hitUV.x);      // 左边缘
        edgeFade *= smoothstep(0.0, fadeDistance, 1.0 - hitUV.x); // 右边缘
        edgeFade *= smoothstep(0.0, fadeDistance, hitUV.y);      // 下边缘
        edgeFade *= smoothstep(0.0, fadeDistance, 1.0 - hitUV.y); // 上边缘
        
        // 混合 SSR 结果与黑色，由 edgeFade 控制透明度
        finalReflectedColor = ssrColor * edgeFade;
    }
    else
    {
        // 如果没有命中屏幕内的几何体，直接返回黑色
        finalReflectedColor = vec3(0.0);
    }
    
    FragColor = vec4(finalReflectedColor, 1.0);
}