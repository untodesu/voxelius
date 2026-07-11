struct AtlasFrame {
    float2 uv_min;
    float2 uv_max;
    uint layer;
    uint pad;
};

struct FragmentInput {
    float2 uv : TEXCOORD0;
    uint frame_index : TEXCOORD1;
    uint tint_index : TEXCOORD2;
    float shade : TEXCOORD3;
};

Texture2DArray t_albedo : register(t0, space2);
SamplerState s_albedo : register(s0, space2);

StructuredBuffer<AtlasFrame> t_gpu_frames : register(t1, space2);

float4 main(FragmentInput input) : SV_TARGET
{
    AtlasFrame frame = t_gpu_frames[input.frame_index];
    float2 final_uv = lerp(frame.uv_min, frame.uv_max, input.uv);
    float4 albedo = t_albedo.Sample(s_albedo, float3(final_uv, float(frame.layer)));
    return float4(albedo.rgb * input.shade, albedo.a);
}
