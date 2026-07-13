struct AtlasFrame_GPU {
    float2 uv_min;
    float2 uv_max;
    uint layer;
    uint resv;
};

struct FragmentInput {
    float2 uv           : TEXCOORD0;
    uint frame_index    : TEXCOORD1;
    uint tint_index     : TEXCOORD2;
    float shade         : TEXCOORD3;
};

Texture2DArray albedo_texture : register(t0, space2);
SamplerState albedo_sampler : register(s0, space2);

StructuredBuffer<AtlasFrame_GPU> gpu_frames : register(t1, space2); // block_atlas::gpu_frames

float4 main(FragmentInput input) : SV_TARGET
{
    AtlasFrame_GPU frame = gpu_frames[input.frame_index];
    float2 final_uv = lerp(frame.uv_min, frame.uv_max, input.uv);
    float4 albedo = albedo_texture.Sample(albedo_sampler, float3(final_uv, float(frame.layer)));
    return float4(albedo.rgb * input.shade, albedo.a);
}
