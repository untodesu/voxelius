#version 330 core

out vec2 vs_TexCoord;

void main(void)
{
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID & 2) << 1);
    gl_Position = vec4(x, y, 0.0, 1.0);
    vs_TexCoord = gl_Position.xy * 0.5 + 0.5;
}
