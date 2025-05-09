#version 450 core

// 输入属性 (来自顶点着色器)
in VS_OUT { // 块名必须与顶点着色器中的 out 块名相同
    vec3 fragPos;   // 变量名必须与顶点着色器 out 块中的变量名相同
    vec3 normal;    // 变量名必须与顶点着色器 out 块中的变量名相同
    vec2 texCoords; // 变量名必须与顶点着色器 out 块中的变量名相同
} fs_in; // 实例名可以不同

// 输出到不同的颜色附件
layout(location = 0) out vec4 out_Position;    // 写入到 GL_COLOR_ATTACHMENT0
// layout(location = 1) out vec4 out_Normal;      // 写入到 GL_COLOR_ATTACHMENT1
// layout(location = 2) out vec4 out_AlbedoSpecular; // 写入到 GL_COLOR_ATTACHMENT2
// layout(location = 3) out vec4 out_RoughnessMetallicAO; // 写入到 GL_COLOR_ATTACHMENT3

void main() {

    // 假设 fragPos 的 x, y, z 分量在一个大致的范围内，例如 -10.0 到 10.0
    // 你需要根据你的场景调整 divisor 的值
    float divisor = 2.0;
    vec3 normalizedPos = (fs_in.fragPos + divisor * 0.5) / divisor; // 将范围映射到 0.0 到 1.0

    out_Position = vec4(normalizedPos, 1.0);
    // 计算位置信息
    // out_Position = vec4(fs_in.fragPos, 1.0);

    // 计算法线信息
    // out_Normal = vec4(normalize(fragNormal), 0.0);

    // 获取反照率和镜面反射强度 (假设从纹理采样)
    // vec4 albedoMap = texture(albedoTexture, fragTexCoords);
    // float specularFactor = 0.5; // 示例镜面反射强度
    // out_AlbedoSpecular = vec4(albedoMap.rgb, specularFactor);

    // 获取粗糙度、金属度和环境光遮蔽 (假设从纹理采样)
    // vec4 rmaMap = texture(rmaTexture, fragTexCoords);
    // out_RoughnessMetallicAO = rmaMap;
}