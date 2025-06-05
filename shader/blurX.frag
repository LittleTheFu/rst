#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D image;
uniform vec2 texelSize;

void main()
{
    float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

    vec3 result = texture(image, TexCoords).rgb * weights[0];
    for (int i = 1; i < 5; ++i)
    {
        result += texture(image, TexCoords + vec2(texelSize.x * i, 0.0)).rgb * weights[i];
        result += texture(image, TexCoords - vec2(texelSize.x * i, 0.0)).rgb * weights[i];
    }

    FragColor = vec4(result, 1.0);
}
