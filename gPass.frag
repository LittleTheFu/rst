#version 450 core

// 输入属性 (来自顶点着色器)
in VS_OUT { // 块名必须与顶点着色器中的 out 块名相同
    vec3 fragPos;   // 变量名必须与顶点着色器 out 块中的变量名相同
    vec3 normal;    // 变量名必须与顶点着色器 out 块中的变量名相同
    vec2 texCoords; // 变量名必须与顶点着色器 out 块中的变量名相同
} fs_in; // 实例名可以不同

// 输出到不同的颜色附件
layout(location = 0) out vec4 out_Position;    // 写入到 GL_COLOR_ATTACHMENT0
layout(location = 1) out vec4 out_Normal;      // 写入到 GL_COLOR_ATTACHMENT1
layout(location = 2) out vec4 out_Albedo; // 写入到 GL_COLOR_ATTACHMENT2
layout(location = 3) out vec4 out_Roughness; // 写入到 GL_COLOR_ATTACHMENT3
layout(location = 4) out vec4 out_Metallic;  // 写入到 GL_COLOR_ATTACHMENT4
layout(location = 5) out vec4 out_Ao;        // 写入到 GL_COLOR_ATTACHMENT5


// 纹理采样器
uniform sampler2D albedoMap;  // 单独声明
uniform bool hasAlbedoMap;    // 也可以单独声明（或留在 UBO 里）

uniform sampler2D normalMap;  // 单独声明
uniform bool hasNormalMap;    // 也可以单独声明（或留在 UBO 里）

uniform sampler2D metallicMap;  // 单独声明
uniform bool hasMetallicMap;    // 也可以单独声明（或留在 UBO 里）

uniform sampler2D roughnessMap;      // 单独声明
uniform bool hasRoughnessMap;        // 也可以单独声明（或留在 UBO 里）

uniform sampler2D aoMap;       // 单独声明
uniform bool hasAoMap;        // 也可以单独声明（或留在 UBO 里）

void main() 
{
    // 写入位置信息到第一个颜色附件
    out_Position = vec4(fs_in.fragPos, 1.0);

    // 计算法线信息
    // out_Normal = vec4(normalize(fs_in.normal), 0.0);
    out_Normal = texture(normalMap, fs_in.texCoords);

    out_Albedo = texture(albedoMap, fs_in.texCoords); // 假设 albedoTexture 是一个纹理采样器

    // 获取反照率和镜面反射强度 (假设从纹理采样)
    out_Ao = texture(aoMap, fs_in.texCoords);

    // 获取粗糙度和金属度 (假设从纹理采样)
    out_Roughness = texture(roughnessMap, fs_in.texCoords); // 假设 rmaMap 是一个纹理采样器
    out_Metallic = texture(metallicMap, fs_in.texCoords); // 假设 metallicMap 是一个纹理采样器
}