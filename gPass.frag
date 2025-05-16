#version 450 core

in VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 texCoords;
    mat3 TBN;
    vec4 clipPos;
    float clipW; // 接收 clipW
} fs_in;

uniform sampler2D albedoMap;
uniform bool hasAlbedoMap;
uniform sampler2D normalMap;
uniform bool hasNormalMap;
uniform sampler2D metallicMap;
uniform bool hasMetallicMap;
uniform sampler2D roughnessMap;
uniform bool hasRoughnessMap;
uniform sampler2D aoMap;
uniform bool hasAoMap;

layout(location = 0) out vec4 out_Position;
layout(location = 1) out vec4 out_Normal;
layout(location = 2) out vec4 out_Albedo;
layout(location = 3) out vec4 out_Roughness;
layout(location = 4) out vec4 out_Metallic;
layout(location = 5) out vec4 out_Ao;

void main()
{
    // out_Position.r = fs_in.clipW;
    // out_Position.g = fs_in.clipW;
    // out_Position.b = fs_in.clipW;
    // out_Position.a = 1.0;

    out_Position = vec4(fs_in.fragPos, 1.0);

    vec4 normalMapValue = texture(normalMap, fs_in.texCoords);
    vec3 normal = normalize(fs_in.TBN * (normalMapValue.xyz * 2.0 - 1.0));
    out_Normal = vec4(normal, 1.0);
    out_Albedo = texture(albedoMap, fs_in.texCoords);
    out_Ao = texture(aoMap, fs_in.texCoords);
    out_Roughness = texture(roughnessMap, fs_in.texCoords);
    out_Metallic = texture(metallicMap, fs_in.texCoords);

    float ndcZ = fs_in.clipPos.z / fs_in.clipPos.w;
    float depthValue = (ndcZ + 1.0) / 2.0;
    gl_FragDepth = depthValue;
}