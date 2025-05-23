#version 450 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D lightTexture;
uniform sampler2D iblTexture;
uniform sampler2D lightDepthTexture;

void main()
{
    // FragColor = mix(skyboxColor, lightColor, lightColor.a);
    // FragColor = texture(screenTexture, TexCoords);

    vec4 lightColor = texture(lightTexture, TexCoords);
    vec4 iblColor = texture(iblTexture, TexCoords);
    // 读取深度纹理
    float depth = texture(lightDepthTexture, TexCoords).r;

    // 使用 lightTexture 的 alpha 通道作为混合因子
    // FragColor = mix(skyColor, lightColor, lightColor.a);
    // float farPlaneThreshold = 0.99999;
    // if (depth > farPlaneThreshold) 
    // {
    //     FragColor = skyColor;
    // }
    // else
    // {
    //     FragColor = lightColor;
    // }

    // FragColor = lightColor;
    FragColor = lightColor + iblColor;
}