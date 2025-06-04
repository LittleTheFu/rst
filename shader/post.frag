#version 450 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D colorTexture;

void main()
{
    vec4 color = texture(colorTexture, TexCoords);

    vec4 invertColor = vec4(1.0 - color.rgb, color.a);

    float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    vec4 grayColor = vec4(gray, gray, gray, color.a);

    vec3 corrected = pow(color.rgb, vec3(1.0/2.2));
    vec4 correctedColor = vec4(corrected, 1.0);

    FragColor = correctedColor;
}