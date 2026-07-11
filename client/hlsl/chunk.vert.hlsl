struct VertexInput {
    uint position : POSITION;
    uint normal : NORMAL;
    float2 uv : TEXCOORD0;
    uint frame_base : TEXCOORD1;
    uint frame_count : TEXCOORD2;
    uint tint_index : TEXCOORD3;
    float shade : TEXCOORD4;
};

struct VertexOutput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    uint frame_index : TEXCOORD1;
    uint tint_index : TEXCOORD2;
    float shade : TEXCOORD3;
};

cbuffer Uniforms_PerFrame : register(b0, space1) {
    float4x4 u_ViewProjection;
    uint u_AnimationTimer;
};

cbuffer Uniforms_PerChunk : register(b1, space1) {
    float3 u_WorldPosition;
};

float3 unpack_position(uint packed)
{
    int x = int(packed & 0x3FFU) - 16;
    int y = int((packed >> 10U) & 0x3FFU) - 16;
    int z = int((packed >> 20U) & 0x3FFU) - 16;
    return float3(x, y, z) / 16.0;
}

VertexOutput main(VertexInput input)
{
    VertexOutput output;

    float3 local_position = unpack_position(input.position) + u_WorldPosition;
    output.position = mul(u_ViewProjection, float4(local_position, 1.0));

    uint frame_count = max(input.frame_count, 1U);
    output.frame_index = input.frame_base + (u_AnimationTimer % frame_count);

    output.uv = input.uv;
    output.tint_index = input.tint_index;
    output.shade = input.shade;

    return output;
}
