#version 330 core

out vec2 vs_TexCoord;

void main(void)
{
    gl_Position.x = -1.0 + float((gl_VertexID & 1) << 2);
    gl_Position.y = -1.0 + float((gl_VertexID & 2) << 1);
    gl_Position.z = 0.0;
    gl_Position.w = 1.0;

    vs_TexCoord.x = gl_Position.x * 0.5 + 0.5;
    vs_TexCoord.y = gl_Position.y * 0.5 + 0.5;
}
