#define SHADE_BIT       0x40000000U
#define UV_ORIENT_BIT   0x80000000U
#define ANIMATED_BIT    0x40000000U

struct VertexInput {
    uint data_origin    : TEXCOORD0;    // Quad origin plus SHADE_BIT and UV_ORIENT_BIT
    uint data_edge_u    : TEXCOORD1;    // Quad edge u plus ANIMATED_BIT
    uint data_edge_v    : TEXCOORD2;    // Quad edge v
    uint data_uv        : TEXCOORD3;    // UV coordinates as unorm8x2
    uint data_texture   : TEXCOORD4;    // Texture index, frame offset and tint index
    uint vertex_id      : SV_VertexID;
};

struct VertexOutput {
    float4 position     : SV_Position;
    float2 uv           : TEXCOORD0;
    uint frame_index    : TEXCOORD1;
    uint tint_index     : TEXCOORD2;
    float shade         : TEXCOORD3;
};

struct AtlasStrip_GPU {
    uint frame_base;
    uint frame_count;
};

StructuredBuffer<AtlasStrip_GPU> gpu_strips : register(t0, space0); // block_atlas::gpu_strips

cbuffer Uniforms_PerFrame : register(b0, space1) {
    float4x4 u_ViewProjection;
    uint u_AnimationTimer;
};

cbuffer Uniforms_PerChunk : register(b1, space1) {
    float3 u_WorldPosition;
};

float3 unpack_position(uint data)
{
    int xpos = int(data & 0x3FFU) - 16;
    int ypos = int((data >> 10U) & 0x3FFU) - 16;
    int zpos = int((data >> 20U) & 0x3FFU) - 16;
    return float3(xpos, ypos, zpos) / 16.0;
}

float3 unpack_offset(uint data)
{
    int dx = int(data & 0x3FFU) - 512;
    int dy = int((data >> 10U) & 0x3FFU) - 512;
    int dz = int((data >> 20U) & 0x3FFU) - 512;
    return float3(dx, dy, dz) / 16.0;
}

float4 unpack_unorm8x4(uint data)
{
    float lo_x = float(data & 0xFFU) / 255.0;
    float lo_y = float((data >> 8U) & 0xFFU) / 255.0;
    float hi_x = float((data >> 16U) & 0xFFU) / 255.0;
    float hi_y = float((data >> 24U) & 0xFFU) / 255.0;
    return float4(lo_x, lo_y, hi_x, hi_y);
}

float shade_factor(float3 normal)
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

VertexOutput main(VertexInput input)
{
    const float2 vertices[6] = {
        float2(0.0, 0.0),
        float2(1.0, 0.0),
        float2(1.0, 1.0),
        float2(0.0, 0.0),
        float2(1.0, 1.0),
        float2(0.0, 1.0),
    };

    float3 origin = unpack_position(input.data_origin);
    float3 edge_u = unpack_offset(input.data_edge_u);
    float3 edge_v = unpack_offset(input.data_edge_v);
    float2 corner_st = vertices[input.vertex_id];

    float3 local_position = origin + corner_st.x * edge_u + corner_st.y * edge_v + u_WorldPosition;

    float4 cx = unpack_unorm8x4(input.data_uv);
    float2 c0 = float2(cx.x, cx.y);
    float2 c2 = float2(cx.z, cx.w);

    uint texture_index = input.data_texture & 0xFFFFU;
    uint frame_offset = (input.data_texture >> 16U) & 0xFFU;
    uint tint_index = (input.data_texture >> 24U) & 0xFFU;
    AtlasStrip_GPU strip = gpu_strips[texture_index];

    VertexOutput output;
    output.position = mul(u_ViewProjection, float4(local_position, 1.0));

    if(input.data_origin & UV_ORIENT_BIT) {
        float u = lerp(c0.x, c2.x, corner_st.y);
        float v = lerp(c0.y, c2.y, corner_st.x);
        output.uv = float2(u, v);
    }
    else {
        float u = lerp(c0.x, c2.x, corner_st.x);
        float v = lerp(c0.y, c2.y, corner_st.y);
        output.uv = float2(u, v);
    }

    if(input.data_edge_u & ANIMATED_BIT) {
        output.frame_index = strip.frame_base + u_AnimationTimer % max(strip.frame_count, 1U);
    }
    else {
        output.frame_index = strip.frame_base + frame_offset;
    }

    output.tint_index = tint_index;

    if(input.data_origin & SHADE_BIT) {
        output.shade = shade_factor(normalize(cross(edge_u, edge_v)));
    }
    else {
        output.shade = 1.0;
    }

    return output;
}
