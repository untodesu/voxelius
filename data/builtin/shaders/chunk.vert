#version 330 core

#define SHADE_BIT       0x40000000U
#define UV_ORIENT_BIT   0x80000000U
#define ANIMATED_BIT    0x40000000U

layout(location = 0) in uint vert_Origin;
layout(location = 1) in uint vert_EdgeU;
layout(location = 2) in uint vert_EdgeV;
layout(location = 3) in uint vert_TexCoord;
layout(location = 4) in uint vert_Texture;
layout(location = 5) in uint vert_Extras;

out vec2 vs_TexCoord;
flat out uint vs_FrameIndex;
flat out uint vs_TintIndex;
out float vs_Shade;
out float vs_AO;

uniform usamplerBuffer u_AtlasStrips;
uniform mat4 u_ViewProjection;
uniform uint u_AnimationTimer;
uniform vec3 u_WorldPosition;

vec3 unpack_position(uint data)
{
    int xpos = int(data & 0x3FFU) - 16;
    int ypos = int((data >> 10U) & 0x3FFU) - 16;
    int zpos = int((data >> 20U) & 0x3FFU) - 16;
    return vec3(xpos, ypos, zpos) / 16.0;
}

vec3 unpack_offset(uint data)
{
    int dx = int(data & 0x3FFU) - 512;
    int dy = int((data >> 10U) & 0x3FFU) - 512;
    int dz = int((data >> 20U) & 0x3FFU) - 512;
    return vec3(dx, dy, dz) / 16.0;
}

vec4 unpack_unorm8x4(uint data)
{
    float lo_x = float(data & 0xFFU) / 255.0;
    float lo_y = float((data >> 8U) & 0xFFU) / 255.0;
    float hi_x = float((data >> 16U) & 0xFFU) / 255.0;
    float hi_y = float((data >> 24U) & 0xFFU) / 255.0;
    return vec4(lo_x, lo_y, hi_x, hi_y);
}

float shade_factor(vec3 normal)
{
    float ax = abs(normal.x);
    float ay = abs(normal.y);
    float az = abs(normal.z);

    if(ay >= ax && ay >= az) {
        if(normal.y >= 0.0) {
            return 1.0;
        }
        else {
            return 0.4;
        }
    }

    if(ax >= az) {
        return 0.6;
    }

    return 0.8;
}

void main(void)
{
    const vec2 vertices_norm[6] = vec2[6](vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 1.0));
    const vec2 vertices_flip[6] = vec2[6](vec2(1.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 1.0), vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(0.0, 0.0));

    vec3 origin = unpack_position(vert_Origin);
    vec3 edge_u = unpack_offset(vert_EdgeU);
    vec3 edge_v = unpack_offset(vert_EdgeV);
    
    uint ao_0 = (vert_Extras >> 0U) & 0x03U;
    uint ao_1 = (vert_Extras >> 2U) & 0x03U;
    uint ao_2 = (vert_Extras >> 4U) & 0x03U;
    uint ao_3 = (vert_Extras >> 6U) & 0x03U;
    bool flip_quad = bool((ao_0 + ao_2) < (ao_1 + ao_3));

    vec2 corner_st = flip_quad ? vertices_flip[gl_VertexID] : vertices_norm[gl_VertexID];

    vec3 local_position = origin + corner_st.x * edge_u + corner_st.y * edge_v + u_WorldPosition;

    vec4 cx = unpack_unorm8x4(vert_TexCoord);
    vec2 c0 = vec2(cx.x, cx.y);
    vec2 c2 = vec2(cx.z, cx.w);

    uint texture_index = vert_Texture & 0xFFFFu;
    uint frame_offset = (vert_Texture >> 16u) & 0xFFU;
    uint tint_index = (vert_Texture >> 24u) & 0xFFU;
    
    uvec4 strip_data = texelFetch(u_AtlasStrips, int(texture_index));
    uint strip_frame_base = strip_data.x;
    uint strip_frame_count = strip_data.y;

    gl_Position = u_ViewProjection * vec4(local_position, 1.0);

    if(bool(vert_Origin & UV_ORIENT_BIT)) {
        float u = mix(c0.x, c2.x, corner_st.y);
        float v = mix(c0.y, c2.y, corner_st.x);
        vs_TexCoord = vec2(u, v);
    }
    else {
        float u = mix(c0.x, c2.x, corner_st.x);
        float v = mix(c0.y, c2.y, corner_st.y);
        vs_TexCoord = vec2(u, v);
    }

    if(bool(vert_EdgeU & ANIMATED_BIT)) {
        vs_FrameIndex = strip_frame_base + u_AnimationTimer % max(strip_frame_count, 1U);
    }
    else {
        vs_FrameIndex = strip_frame_base + frame_offset;
    }

    vs_TintIndex = tint_index;

    if(bool(vert_Origin & SHADE_BIT)) {
        vs_Shade = shade_factor(normalize(cross(edge_u, edge_v)));
    }
    else {
        vs_Shade = 1.0;
    }

    uint ao_values[4] = uint[4](ao_0, ao_1, ao_3, ao_2);
    uint corner_idx = uint(corner_st.x) + uint(corner_st.y) * 2U;
    float ao_factor = float(ao_values[corner_idx]) / 3.0;
    vs_AO = mix(0.1, 1.0, ao_factor);
}
