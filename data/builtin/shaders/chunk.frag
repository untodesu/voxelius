#version 330 core

in vec2 vs_TexCoord;
flat in uint vs_FrameIndex;
flat in uint vs_TintIndex;
in float vs_Shade;

out vec4 frag_ColorTarget;

uniform sampler2DArray u_AtlasTexture;
uniform samplerBuffer u_AtlasFrames;

struct AtlasFrame {
    vec2 uv_min;
    vec2 uv_max;
    uint layer;
};

AtlasFrame get_frame(int index) {
    vec4 part1 = texelFetch(u_AtlasFrames, index * 2);
    vec4 part2 = texelFetch(u_AtlasFrames, index * 2 + 1);

    AtlasFrame frame;
    frame.uv_min = part1.xy;
    frame.uv_max = part1.zw;
    frame.layer = floatBitsToUint(part2.x);
    
    return frame;
}

void main() {
    AtlasFrame frame = get_frame(int(vs_FrameIndex));
    vec2 final_uv = mix(frame.uv_min, frame.uv_max, vs_TexCoord);
    vec4 albedo = texture(u_AtlasTexture, vec3(final_uv, float(frame.layer)));
    frag_ColorTarget = vec4(albedo.rgb * vs_Shade, albedo.a);
}
