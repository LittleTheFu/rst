#version 460 core

in vec2 vTexCoords;
out vec4 FragColor;

uniform sampler2D gNormal;
uniform sampler2D gDepth;
uniform sampler2D sceneColor;
uniform sampler2D gMetallic;
uniform sampler2D gRoughness;

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;
uniform vec2 uScreenSize;

// 反投影，重建 view 空间位置
vec3 reconstructViewPos(vec2 uv, float depth)
{
    float z = depth * 2.0 - 1.0;
    vec4 clip = vec4(uv * 2.0 - 1.0, z, 1.0);
    vec4 view = inverse(uProjectionMatrix) * clip;
    return view.xyz / view.w;
}

vec3 reflectDirection(vec3 viewDir, vec3 normal)
{
    return reflect(viewDir, normal);
}

void main()
{
    float depth = texture(gDepth, vTexCoords).r;
    if (depth >= 1.0) {
        FragColor = vec4(0.0);
        return;
    }

    float metallic = texture(gMetallic, vTexCoords).r;
    if (metallic < 0.5) {
        FragColor = vec4(0.0);
        return; 
    }

    float roughness = texture(gRoughness, vTexCoords).r;
    if (roughness > 0.3) {
        FragColor = vec4(0.0);
        return;
    }


    vec3 normal = normalize(texture(gNormal, vTexCoords).xyz * 2.0 - 1.0);
    vec3 viewPos = reconstructViewPos(vTexCoords, depth);
    vec3 viewDir = normalize(-viewPos);
    vec3 reflDir = reflectDirection(viewDir, normal);

    // 屏幕空间步进准备
    vec3 ray = reflDir;
    vec3 origin = viewPos;

    const int maxSteps = 64;
    const float stepSize = 0.1;

    vec3 hitPos = origin;
    vec2 hitUV = vec2(0.0);
    bool hit = false;

    for (int i = 0; i < maxSteps; ++i)
    {
        hitPos += ray * stepSize;

        vec4 proj = uProjectionMatrix * vec4(hitPos, 1.0);
        proj.xy /= proj.w;
        hitUV = proj.xy * 0.5 + 0.5;

        if (hitUV.x < 0.0 || hitUV.x > 1.0 || hitUV.y < 0.0 || hitUV.y > 1.0)
            break;

        float sceneDepth = texture(gDepth, hitUV).r;
        vec3 scenePos = reconstructViewPos(hitUV, sceneDepth);
        float dist = length(scenePos - hitPos);

        if (dist < 0.05)
        {
            hit = true;
            break;
        }
    }

    if (hit)
    {
        vec3 reflectedColor = texture(sceneColor, hitUV).rgb;
        FragColor = vec4(reflectedColor, 1.0);
    }
    else
    {
        FragColor = vec4(0.0); // 可以用 environment 替代
    }
}
