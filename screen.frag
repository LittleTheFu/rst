#version 450 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D positionTexture;
uniform sampler2D normalTexture;
uniform sampler2D albedoTexture;
uniform sampler2D metallicTexture;
uniform sampler2D roughnessTexture;
uniform sampler2D aoTexture;


layout (std140) uniform PointLightBlock {
    vec3 position;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
} uPointLight;

void main()
{
 FragColor = texture(positionTexture, TexCoords);
//  FragColor = PointLightBlock.color;
    // FragColor = vec4(uPointLight.color, 1.0);

}