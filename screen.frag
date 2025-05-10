#version 450 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D positionTexture;
uniform sampler2D normalTexture;
uniform sampler2D albedoTexture;
uniform sampler2D metallicTexture;
uniform sampler2D roughnessTexture;
uniform sampler2D aoTexture;

void main()
{
 FragColor = texture(positionTexture, TexCoords);
}