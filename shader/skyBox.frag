#version 330 core
in vec3 TexCoords;
out vec4 FragColor;

uniform samplerCube cubeMap;

void main() {
    FragColor = texture(cubeMap, TexCoords);
    //  FragColor = vec4(TexCoords, 1);
}
