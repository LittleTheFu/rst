#version 450 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D lightTexture;
uniform sampler2D iblTexture;
uniform sampler2D gpassDepthTexture;
uniform sampler2D accumTexture;
uniform sampler2D revealTexture;
uniform sampler2D skyboxTexture;
uniform sampler2D ssrTexture;

void main()
{
    // FragColor = mix(skyboxColor, lightColor, lightColor.a);
    // FragColor = texture(screenTexture, TexCoords);

    vec4 ssrColor = texture(ssrTexture, TexCoords);
    vec4 lightColor = texture(lightTexture, TexCoords);
    vec4 iblColor = texture(iblTexture, TexCoords);
    // 读取深度纹理
    float depth = texture(gpassDepthTexture, TexCoords).r;

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
    // FragColor = lightColor + iblColor;
    // FragColor = iblColor;

    vec4 gPassFinalColor = lightColor * 0.7 + iblColor * 0.1 + ssrColor * 0.2;

    //patch: sky box
    if(depth >= 1.0)
    {
        gPassFinalColor = texture(skyboxTexture, TexCoords);
    }

    vec4 accum = texture(accumTexture, TexCoords);
    float reveal = texture(revealTexture, TexCoords).r;

    float alpha = accum.a;
    vec3 color = accum.rgb / max(alpha, 1e-5);
    vec4 oitColor = vec4(color, 1.0 - reveal);

    // FragColor = lightColor;
    FragColor = oitColor * 1.0 + gPassFinalColor;
    // FragColor = mix(gFinalColor, oitColor, oitColor.a);
}