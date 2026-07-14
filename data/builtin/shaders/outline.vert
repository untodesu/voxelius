#version 330 core

layout(location = 0) in vec3 vert_Position;

uniform mat4 u_ViewProjection;
uniform vec3 u_WorldPosition;
uniform float u_ViewDistance;
uniform vec3 u_Scale;

void main(void)
{
    gl_Position = u_ViewProjection * vec4((u_WorldPosition + u_Scale * vert_Position), 1.0);
    gl_Position.z -= 0.00025 * gl_Position.w / pow(max(1.0, length(gl_Position.xyz)), 2.0);
}
