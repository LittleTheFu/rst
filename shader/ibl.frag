#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

// G-Buffer 纹理
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo; // 包含了基色 albedo
uniform sampler2D gRoughness;
uniform sampler2D gMetallic;
uniform sampler2D gAO;

// IBL 预计算纹理
uniform samplerCube irradianceMap;   // 辐照度图 (间接漫反射)
uniform samplerCube prefilterMap;    // 预过滤环境贴图 (间接镜面反射)
uniform sampler2D brdfLUT;           // BRDF 积分查找表 (间接镜面反射)

uniform vec3 camPos;
uniform float maxReflectionLOD; // 从 C++ 传递过来的最大 LOD 级别

// 辅助函数 (BRDF 部分，与你的 PBR shader 中的可能相同)
const float PI = 3.14159265359;

// 菲涅尔-史力克近似，带粗糙度
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ----------------------------------------------------------
// 主函数
void main()
{
    // 从 G-Buffer 采样
    vec3 FragPos    = texture(gPosition, TexCoords).rgb;
    vec3 Normal     = texture(gNormal, TexCoords).rgb;
    vec3 Albedo     = texture(gAlbedo, TexCoords).rgb; // 基色
    float Roughness = texture(gRoughness, TexCoords).r;
    float Metallic  = texture(gMetallic, TexCoords).r;
    float AO        = texture(gAO, TexCoords).r;

    // 确保法线是单位向量
    Normal = normalize(Normal);
    // 计算观察向量
    vec3 ViewDir = normalize(camPos - FragPos);

    // 计算菲涅尔 F0 (基础菲涅尔反射率)
    vec3 F0 = vec3(0.04); // 默认非金属F0
    F0 = mix(F0, Albedo, Metallic); // 对于金属，F0 是 albedo

    // ----------------------------------------------------------
    // 间接漫反射 (Diffuse IBL)
    vec3 irradiance = texture(irradianceMap, Normal).rgb; // 根据法线采样辐照度图
    vec3 diffuseIBL = irradiance * Albedo; // 漫反射部分

    // ----------------------------------------------------------
    // 间接镜面反射 (Specular IBL)
    vec3 R = reflect(-ViewDir, Normal); // 计算反射向量

    // 根据粗糙度采样预过滤环境贴图
    float mipLevel = Roughness * maxReflectionLOD; // 映射粗糙度到 LOD 级别
    vec3 prefilteredColor = textureLod(prefilterMap, R, mipLevel).rgb;

    // 从 BRDF LUT 采样 scale 和 bias 值
    vec2 envBRDF = texture(brdfLUT, vec2(max(dot(Normal, ViewDir), 0.0), Roughness)).rg;

    // 应用 Split Sum 近似中的 Fresnel 和 BRDF LUT 结果
    vec3 F_IBL = FresnelSchlickRoughness(max(dot(Normal, ViewDir), 0.0), F0, Roughness);
    vec3 specularIBL = prefilteredColor * (F_IBL * envBRDF.x + envBRDF.y);

    // ----------------------------------------------------------
    // 最终组合 (IBL 部分)
    // 根据金属度混合间接漫反射和间接镜面反射
    vec3 kS = F_IBL; // 菲涅尔项作为镜面反射系数
    vec3 kD = vec3(1.0) - kS; // 漫反射系数
    kD *= (1.0 - Metallic); // 如果是金属，则没有漫反射

    vec3 ambient = (kD * diffuseIBL + specularIBL) * AO; // 结合 AO (环境光遮蔽)

    // 输出 IBL 结果 (这是一个线性空间的 HDR 值)
    FragColor = vec4(ambient, 1.0);
}