#version 450 core

in vec2 TexCoords;
out vec4 FragColor;

// G-buffer 输入纹理
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gRoughness;
uniform sampler2D gMetallic;
uniform sampler2D gAo;

// 光源信息 (这里只包含一个点光源作为示例)
uniform vec3 lightPos;
uniform vec3 lightColor;

// 相机位置 (用于计算观察方向)
uniform vec3 cameraPos;

// 环境光强度
uniform float ambientStrength = 0.1;

// PBR 相关函数 (可以放在一个 include 文件中以保持 Shader 清洁)
vec3 lerp(vec3 a, vec3 b, float f) {
    return a + f * (b - a);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float den = (NdotH2 * (a2 - 1.0) + 1.0);
    den = max(den * den * 3.14159, 0.0001); // 避免除以零
    return num / den;
}

float SchlickGGX(float NdotV, float roughness)
{
    float a = roughness;
    float k = (a * a) / 2.0;
    float F0 = 0.04; // 默认的非金属反射率
    return F0 + (1.0 - F0) * pow(1.0 - NdotV, 5.0);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    // 从 G-buffer 采样
    vec3 worldPos = texture(gPosition, TexCoords).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoords).xyz);
    vec3 albedo = texture(gAlbedo, TexCoords).rgb;
    float roughness = texture(gRoughness, TexCoords).r;
    float metallic = texture(gMetallic, TexCoords).r;
    float ao = texture(gAo, TexCoords).r;

    // 计算观察方向
    vec3 viewDir = normalize(cameraPos - worldPos);

    // 计算光照方向
    vec3 lightDir = normalize(lightPos - worldPos);

    // 半程向量
    vec3 halfDir = normalize(lightDir + viewDir);

    // --- 漫反射计算 (简单 Lambertian) ---
    float NdotL = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = albedo * lightColor * NdotL;

    // --- 镜面反射计算 (GGX + Schlick Fresnel) ---
    float D = DistributionGGX(normal, halfDir, roughness);
    float NdotV = max(dot(normal, viewDir), 0.0);
    float F = FresnelSchlick(max(dot(halfDir, viewDir), 0.0), vec3(0.04)); // 使用默认 F0
    float G = SchlickGGX(NdotL, roughness) * SchlickGGX(NdotV, roughness);

    vec3 specular = lightColor * (F * G * D) / max(4.0 * NdotL * NdotV, 0.0001);

    // --- 金属材质处理 ---
    vec3 F0 = lerp(vec3(0.04), albedo, metallic);
    specular = lightColor * FresnelSchlick(max(dot(halfDir, viewDir), 0.0), F0) * G * D / max(4.0 * NdotL * NdotV, 0.0001);
    diffuse = albedo * lightColor * NdotL * (1.0 - FresnelSchlick(max(dot(halfDir, viewDir), 0.0), F0)); // 近似能量守恒

    // --- 环境光 (简单环境色) ---
    vec3 ambient = albedo * ambientStrength;

    // --- 最终颜色 ---
    vec3 color = (ambient + ao * (diffuse + specular));

    FragColor = vec4(color, 1.0);
}