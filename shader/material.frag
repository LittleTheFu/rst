struct Material {
    sampler2D albedoMap;
    bool hasAlbedoMap;
    sampler2D normalMap;
    bool hasNormalMap;
    sampler2D roughnessMap;
    bool hasRoughnessMap;
    sampler2D metallicMap;
    bool hasMetallicMap;
    sampler2D aoMap;
    bool hasAoMap;

    vec3 albedoColor;
    float roughnessFactor;
    float metallicFactor;
};

uniform Material material;

in vec2 TexCoords;

void main() {
    vec3 albedo = material.albedoColor;
    if (material.hasAlbedoMap) {
        albedo = texture(material.albedoMap, TexCoords).rgb;
    }
    // ... 使用其他材质属性进行光照计算 ...
}