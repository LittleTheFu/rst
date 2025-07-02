// assets/shaders/rmlui_vertex.glsl
#version 460 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec4 vColor;
out vec2 vTexCoord;

uniform mat4 projection;
uniform vec2 translation; // RmlUi 提供的平移

void main()
{
    gl_Position = projection * vec4(aPos + translation, 0.0, 1.0);
    vColor = aColor;
    vTexCoord = aTexCoord;
}

