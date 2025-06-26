#version 450 core

in VS_OUT {
    vec2 texCoords;
    vec3 worldPos;
    mat3 TBN;
} fs_in;

layout(location = 0) out vec4 accum;   // 预乘颜色+alpha累计
layout(location = 1) out float reveal; // revealage
// layout(location = 1) out vec4 reveal; // revealage

uniform vec3 albedoColor;
uniform float roughnessFactor;
uniform float metallicFactor;
uniform float aoFactor;

uniform sampler2D albedoMap;
uniform bool hasAlbedoMap;

//Material::bindTextures()
uniform sampler2D albedoTexture;
uniform sampler2D normalTexture;
uniform sampler2D metallicTexture;
uniform sampler2D roughnessTexture;
uniform sampler2D aoTexture;

uniform samplerCube irradianceMap;   // 辐照度图 (间接漫反射)
uniform samplerCube prefilterMap;    // 预过滤环境贴图 (间接镜面反射)
uniform sampler2D brdfLUT;           // BRDF 积分查找表 (间接镜面反射)

uniform float maxReflectionLOD; // 从 C++ 传递过来的最大 LOD 级别

uniform vec3 cameraPos;

layout (std140) uniform PointLightBlock {
    vec3 position;
    float pad0;
    vec3 color;
    float pad1;
    float intensity;
    // float constant;
    // float linear;
    // float quadratic;
} uPointLight;

const float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}


// 菲涅尔-史力克近似，带粗糙度
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    vec3 world_position = fs_in.worldPos;

    vec4 normalMapValue = texture(normalTexture, fs_in.texCoords);
    vec3 normal = normalize(fs_in.TBN * (normalMapValue.xyz * 2.0 - 1.0));

    vec3 albedo = texture(albedoTexture, fs_in.texCoords).rgb * albedoColor;
    // albedo = vec3(1.0);

    // float metallic = texture(metallicTexture, fs_in.texCoords).r;
    float metallic = metallicFactor * 0.0;//zero here

    float roughness = texture(roughnessTexture, fs_in.texCoords).r * roughnessFactor;
    vec3 ao = texture(aoTexture, fs_in.texCoords).rgb * aoFactor;

    // 确保粗糙度在0.05到1.0之间
    roughness = clamp(roughness, 0.05, 1.0);  // 不要为0，也不要大于1

    //bug here ? take care of the view direction~~~
    vec3 viewDir = normalize(cameraPos - world_position);
    vec3 lightDir = normalize(uPointLight.position - world_position);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float distance = length(uPointLight.position - world_position);
    float attenuation = 1.0 / (distance * distance);

    vec3 radiance = uPointLight.color * uPointLight.intensity * attenuation;
    // vec3 radiance = uPointLight.color * uPointLight.intensity;
    // vec3 radiance = uPointLight.color;

    // fresnelSchlick
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
    vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);

    float NDF = DistributionGGX(normal, halfwayDir, roughness);       
    float G = GeometrySmith(normal, viewDir, lightDir, roughness); 

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0)  + 0.0001;
    vec3 specular = numerator / denominator; 

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;

    kD *= 1.0 - metallic;
  
    float NdotL = max(dot(normal, lightDir), 0.0);  

    vec3 Lo = vec3(0.0);      
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;

    vec3 ambient = vec3(0.45) * albedo * ao;
    // vec3 ambient = vec3(0.03) * albedo * ao;
    vec4 pbrColor = vec4(Lo, 1.0) + vec4(ambient, 1.0);

    //-------------------------------------------------------------------------------
        // 从 G-Buffer 采样
    // vec3 FragPos    = texture(gPosition, TexCoords).rgb;
    // vec3 Normal     = texture(gNormal, TexCoords).rgb;
    // vec3 Albedo     = texture(gAlbedo, TexCoords).rgb; // 基色
    // float Roughness = texture(gRoughness, TexCoords).r;
    // float Metallic  = texture(gMetallic, TexCoords).r;
    // float AO        = texture(gAO, TexCoords).r;

    // 确保法线是单位向量
    normal = normalize(normal);
    // 计算观察向量
    // vec3 ViewDir = normalize(camPos - FragPos);

    // 计算菲涅尔 F0 (基础菲涅尔反射率)
    vec3 F0_ = vec3(0.04); // 默认非金属F0
    F0_ = mix(F0_, albedo, metallic); // 对于金属，F0 是 albedo

    // ----------------------------------------------------------
    // 间接漫反射 (Diffuse IBL)
    vec3 irradiance = texture(irradianceMap, normal).rgb; // 根据法线采样辐照度图
    vec3 diffuseIBL = irradiance * albedo; // 漫反射部分

    // ----------------------------------------------------------
    // 间接镜面反射 (Specular IBL)
    vec3 R = reflect(-viewDir, normal); // 计算反射向量

    // 根据粗糙度采样预过滤环境贴图
    float mipLevel = roughness * maxReflectionLOD; // 映射粗糙度到 LOD 级别
    vec3 prefilteredColor = textureLod(prefilterMap, R, mipLevel).rgb;

    // 从 BRDF LUT 采样 scale 和 bias 值
    vec2 envBRDF = texture(brdfLUT, vec2(max(dot(normal, viewDir), 0.0), roughness)).rg;

    // 应用 Split Sum 近似中的 Fresnel 和 BRDF LUT 结果
    vec3 F_IBL = FresnelSchlickRoughness(max(dot(normal, viewDir), 0.0), F0_, roughness);
    vec3 specularIBL = prefilteredColor * (F_IBL * envBRDF.x + envBRDF.y);

    // ----------------------------------------------------------
    // 最终组合 (IBL 部分)
    // 根据金属度混合间接漫反射和间接镜面反射
    vec3 kS_ = F_IBL; // 菲涅尔项作为镜面反射系数
    vec3 kD_ = vec3(1.0) - kS_; // 漫反射系数
    kD_ *= (1.0 - metallic); // 如果是金属，则没有漫反射

    vec3 ambient_ = (kD_ * diffuseIBL + specularIBL) * ao; // 结合 AO (环境光遮蔽)

    // 输出 IBL 结果 (这是一个线性空间的 HDR 值)
    vec4 iblColor = vec4(ambient_, 1.0);

    //-------------------------------------------------------------------------------
    // warning: 争议点，ambient可能多加了
    vec4 color = iblColor + pbrColor;
    // vec4 color = pbrColor;
    // vec4 color = iblColor;
    // vec4 color = texture(albedoMap, fs_in.texCoords);    
    color.a = 0.5;

    float depthFactor = clamp(0.03 / (1e-5 + pow(gl_FragCoord.z / 200.0, 4.0)), 1e-2, 3e3);
    float base = max(max(color.r, color.g), color.b) * color.a;
    float weight = max(base, color.a) * depthFactor;

    // accum = vec4(color.rgb * color.a, color.a) * weight * 0.0001;
    accum = vec4(color.rgb * color.a, color.a) * weight * 0.0001;

    reveal = color.a;
    // reveal = 0.5;    
}