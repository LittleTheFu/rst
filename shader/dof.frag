#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D sharpTexture;
uniform sampler2D blurTexture;
uniform sampler2D depthTexture;

uniform float focusDepth;
uniform float focusRange;
uniform float nearPlane;
uniform float farPlane;

// 从深度值转换为线性深度
float linearizeDepth(float z) {
    float ndc = z * 2.0 - 1.0; // [0,1] -> [-1,1]
    return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - ndc * (farPlane - nearPlane));
}

void main() {
    float z = texture(depthTexture, TexCoords).r;
    float linearDepth = linearizeDepth(z);

    float blurFactor = clamp(abs(linearDepth - focusDepth) / focusRange, 0.0, 1.0);

    vec3 sharp = texture(sharpTexture, TexCoords).rgb;
    vec3 blur = texture(blurTexture, TexCoords).rgb;

    vec3 finalColor = mix(sharp, blur, blurFactor);
    FragColor = vec4(finalColor, 1.0);
}
