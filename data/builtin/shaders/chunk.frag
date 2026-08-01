#version 330 core
#pragma variant 0 FOG_MODEL

in vec2 vs_TexCoord;
flat in uint vs_FrameIndex;
flat in uint vs_MaskFrame;
flat in vec3 vs_TintColor;
in float vs_Shade;
in float vs_AO;

#if FOG_MODEL
in float vs_FogFactor;
#endif

out vec4 frag_Target;

uniform vec3 u_FogColor;
uniform sampler2DArray u_AtlasTexture;
uniform samplerBuffer u_AtlasFrames;

struct AtlasFrame {
    vec2 uv_min;
    vec2 uv_max;
    uint layer;
};

AtlasFrame get_frame(int index)
{
    vec4 part1 = texelFetch(u_AtlasFrames, index * 2);
    vec4 part2 = texelFetch(u_AtlasFrames, index * 2 + 1);

    AtlasFrame frame;
    frame.uv_min = part1.xy;
    frame.uv_max = part1.zw;
    frame.layer = floatBitsToUint(part2.x);
    
    return frame;
}

void main(void)
{
    AtlasFrame frame = get_frame(int(vs_FrameIndex));
    vec2 final_uv = mix(frame.uv_min, frame.uv_max, vs_TexCoord);
    vec4 albedo = texture(u_AtlasTexture, vec3(final_uv, float(frame.layer)));

    AtlasFrame mask_frame = get_frame(int(vs_MaskFrame));
    vec2 mask_uv = mix(mask_frame.uv_min, mask_frame.uv_max, vs_TexCoord);
    float tint_mask = texture(u_AtlasTexture, vec3(mask_uv, float(mask_frame.layer))).r;

    vec3 tint = mix(vec3(1.0), vs_TintColor, tint_mask);
    vec3 shaded = albedo.rgb * vs_Shade * vs_AO * tint;

    frag_Target.rgb = shaded;
    frag_Target.a = albedo.a;

#if FOG_MODEL
    frag_Target.rgb = mix(frag_Target.rgb, u_FogColor, vs_FogFactor);
#endif
}
