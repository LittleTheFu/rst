// assets/shaders/rmlui_fragment.glsl
#version 460 core
out vec4 FragColor;

in vec4 vColor;
in vec2 vTexCoord;

uniform sampler2D uTexture;
uniform bool hasTexture;

void main()
{
    if (hasTexture) {
        FragColor = vColor * texture(uTexture, vTexCoord);
    } else {
        FragColor = vColor;
    }
}
