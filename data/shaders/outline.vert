/* SPDX-License-Identifier: CC-BY-SA-4.0 */
#version 330 core
#pragma variant[0] WORLD_CURVATURE

layout(location = 0) in vec3 vert_Position;

uniform mat4x4 u_ViewProjMatrix;
uniform vec3 u_WorldPosition;
uniform float u_ViewDistance;
uniform vec3 u_Scale;

void main(void)
{
    gl_Position = u_ViewProjMatrix * vec4((u_WorldPosition + u_Scale * vert_Position), 1.0);
    gl_Position.z -= 0.00025;

#if WORLD_CURVATURE
    gl_Position.y -= gl_Position.z * gl_Position.z / u_ViewDistance / 32.0;
    gl_Position.y -= gl_Position.x * gl_Position.x / u_ViewDistance / 32.0;
#endif
}
