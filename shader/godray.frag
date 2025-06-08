#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D brightnessTex;
uniform vec2 lightScreenPos;

uniform float exposure;
uniform float decay;
uniform float density;
uniform float weight;
uniform int numSamples;

void main()
{
    vec2 deltaTexCoord = TexCoords - lightScreenPos;
    deltaTexCoord *= 1.0 / float(numSamples) * density;

    vec2 texCoord = TexCoords;
    float illuminationDecay = 1.0;

    vec4 color = vec4(0.0);

    for (int i = 0; i < numSamples; ++i)
    {
        texCoord -= deltaTexCoord;
        vec4 sample = texture(brightnessTex, texCoord);
        sample *= illuminationDecay * weight;
        color += sample;
        illuminationDecay *= decay;
    }

    color *= exposure;
    FragColor = color;
}
