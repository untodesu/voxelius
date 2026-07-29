#version 330 core
#pragma variant 0 FOG_MODEL

#define ANIMATED_BIT 0x00000004U

layout(location = 0) in uint vert_Position;
layout(location = 1) in uint vert_TexCoord;
layout(location = 2) in uint vert_Texture;
layout(location = 3) in uint vert_Extras;
layout(location = 4) in uint vert_ChunkSlot;

out vec2 vs_TexCoord;
flat out uint vs_FrameIndex;
flat out uint vs_TintIndex;
out float vs_Shade;
out float vs_AO;

#if FOG_MODEL
out float vs_FogFactor;
#endif

uniform usamplerBuffer u_AtlasStrips;
uniform samplerBuffer u_ChunkPositions;
uniform mat4 u_ViewProjection;
uniform uint u_AnimationTimer;
uniform float u_ViewDistance;

vec3 unpack_position(uint data)
{
    int xpos = int(data & 0x3FFU) - 16;
    int ypos = int((data >> 10U) & 0x3FFU) - 16;
    int zpos = int((data >> 20U) & 0x3FFU) - 16;
    return vec3(xpos, ypos, zpos) / 16.0;
}

void main(void)
{
    vec3 local_position = unpack_position(vert_Position) + texelFetch(u_ChunkPositions, int(vert_ChunkSlot)).xyz;

    vs_TexCoord = vec2(float(vert_TexCoord & 0xFFU), float((vert_TexCoord >> 8U) & 0xFFU)) / 255.0;

    uint texture_index = vert_Texture & 0xFFFFu;
    uint frame_offset = (vert_Texture >> 16u) & 0xFFU;
    uint tint_index = (vert_Texture >> 24u) & 0xFFU;

    uvec4 strip_data = texelFetch(u_AtlasStrips, int(texture_index));
    uint strip_frame_base = strip_data.x;
    uint strip_frame_count = strip_data.y;

    gl_Position = u_ViewProjection * vec4(local_position, 1.0);

    if(bool(vert_Extras & ANIMATED_BIT)) {
        vs_FrameIndex = strip_frame_base + u_AnimationTimer % max(strip_frame_count, 1U);
    }

    else {
        vs_FrameIndex = strip_frame_base + frame_offset;
    }

    vs_TintIndex = tint_index;
    vs_Shade = float((vert_Extras >> 8U) & 0xFFU) / 255.0;

    float ao_factor = float(vert_Extras & 0x03U) / 3.0;
    vs_AO = mix(0.25, 1.0, ao_factor);

#if FOG_MODEL == 1
    vs_FogFactor = 1.0 - clamp((u_ViewDistance - length(gl_Position.xyz)) / (u_ViewDistance - 16.0), 0.0, 1.0);
#elif FOG_MODEL == 2
    float fogd = 2.0 / u_ViewDistance * length(gl_Position.xyz);
    vs_FogFactor = 1.0 - clamp(exp2(fogd * fogd * -1.442695), 0.0, 1.0);
#endif
}
