#version 450 core
in vec2 TexCoords;
// out vec4 FragColor;

uniform sampler2D positionTexture;
uniform sampler2D normalTexture;
uniform sampler2D albedoTexture;
uniform sampler2D metallicTexture;
uniform sampler2D roughnessTexture;
uniform sampler2D aoTexture;
uniform samplerCube shadowMapTexture;


uniform vec3 cameraPos;

uniform float shadowCameraFarClip;

layout(location = 0) out vec4 out_Texture;    // 写入到 GL_COLOR_ATTACHMENT0
layout(location = 1) out vec4 out_DebugCurrentDepth;  // 对应 GL_COLOR_ATTACHMENT1
layout(location = 2) out vec4 out_DebugClosestDepth;  // 对应 GL_COLOR_ATTACHMENT2



const float PI = 3.14159265359;


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

void main()
{
    vec3 position = texture(positionTexture, TexCoords).rgb;
    vec3 normal = texture(normalTexture, TexCoords).rgb;
    vec3 albedo = texture(albedoTexture, TexCoords).rgb;
    float metallic = texture(metallicTexture, TexCoords).r;
    float roughness = texture(roughnessTexture, TexCoords).r;
    vec3 ao = texture(aoTexture, TexCoords).rgb;

    // 这里暗含一个bug，因为depth的映射不是线性的。
    vec3 fragToLight = (position - uPointLight.position);
    vec3 fragToLight_norm = normalize(fragToLight);
    // vec3 fragToLight = normalize(position - uPointLight.position);
    float closestDepth = texture(shadowMapTexture, fragToLight_norm).r * shadowCameraFarClip;
    float currentDepth = length(fragToLight);
    float bias = 0.0000; // 尝试加大一些偏移看看效果
    float shadow = 0.0;
    // if (currentDepth > shadowCameraFarClip) {
    if(false){
        shadow = 1.0; // 超出范围，不投影阴影
    } else {
        shadow = (currentDepth - bias > closestDepth) ? 1.0 : 0.0;
    }
    // shadow = 0;

    // float diff = (currentDepth - bias) - closestDepth;

    // 确保粗糙度在0.05到1.0之间
    roughness = clamp(roughness, 0.05, 1.0);  // 不要为0，也不要大于1

    vec3 viewDir = normalize(cameraPos - position);
    vec3 lightDir = normalize(uPointLight.position - position);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float distance = length(uPointLight.position - position);
    float attenuation = 1.0 / (distance * distance);

    // attenuation = 1.0;
    // vec3 radiance = uPointLight.color * 1000 * attenuation;
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
    vec4 outColor = vec4(Lo, 1.0) + vec4(ambient, 1.0);
    //  vec4 outColor = vec4(Lo, 1.0);
    out_Texture = (1 - shadow) * outColor;

    // out_Texture = vec4(shadow, shadow, shadow, 1.0);

    // if(closestDepth > 1)
    // {
    //     closestDepth = 0;
    // }
    currentDepth = currentDepth / shadowCameraFarClip;
    // out_Texture = vec4(currentDepth, currentDepth, currentDepth, 1.0);
    out_DebugCurrentDepth = vec4(currentDepth, currentDepth, currentDepth, 1.0);

    closestDepth = closestDepth / shadowCameraFarClip;
    out_DebugClosestDepth = vec4(closestDepth, closestDepth, closestDepth, 1.0);
    // out_DebugClosestDepth = vec4(fragToLight, 1.0);
    // out_Texture = vec4(closestDepth, closestDepth, closestDepth, 1.0);
    // out_Texture = vec4(texture(shadowMapTexture, fragToLight).r, texture(shadowMapTexture, fragToLight).r, texture(shadowMapTexture, fragToLight).r, 1.0);

    

    // float diff = (currentDepth - bias) - closestDepth;
    // out_Texture = vec4(diff, diff, diff, 1.0);

    // FragColor = vec4(uPointLight.intensity, uPointLight.intensity, uPointLight.intensity, 1.0);
    // out_Texture = vec4(dpth, dpth, dpth, 1.0);
}