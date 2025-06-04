#version 450 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D colorTexture;

void main()
{
    FragColor = texture(colorTexture, TexCoords);
}