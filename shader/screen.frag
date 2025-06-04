#version 450 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D colorTexture;

void main()
{
    vec4 color = texture(colorTexture, TexCoords);

    FragColor = color;
}