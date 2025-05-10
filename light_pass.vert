#version 450 core

out vec2 TexCoords;

void main()
{
    // 使用裁剪空间坐标来覆盖整个屏幕
    vec2 ndc = vec2((gl_VertexID == 0 || gl_VertexID == 2) ? -1.0 : 1.0,
                    (gl_VertexID == 1 || gl_VertexID == 2) ? -1.0 : 1.0);
    TexCoords = ndc * 0.5 + 0.5; // 将 NDC 坐标转换为 [0, 1] 的 UV 坐标
    gl_Position = vec4(ndc, 0.0, 1.0);
}