#version 450 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D colorTexture;

void main()
{
    vec3 color = texture(colorTexture, TexCoords).rgb;

    // vec4 invertColor = vec4(1.0 - color.rgb, color.a);

    // float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    // vec4 grayColor = vec4(gray, gray, gray, color.a);

    //tonemap
    color = color / (color + vec3(1.0));

    //gamma correct
    color = pow(color, vec3(1.0/2.2));
    vec4 outputColor = vec4(color, 1.0);

    FragColor = outputColor;
}