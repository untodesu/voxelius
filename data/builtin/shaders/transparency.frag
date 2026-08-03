#version 330 core
#pragma variant 0 FOG_MODEL

in vec2 vs_TexCoord;

out vec4 frag_Target;

uniform sampler2D u_DiffuseColor;
uniform sampler2D u_DiffuseDepth;
uniform sampler2D u_AlphaColor;
uniform sampler2D u_AlphaDepth;
uniform sampler2D u_FluidColor;
uniform sampler2D u_FluidDepth;

uniform vec3 u_FogColor;
uniform float u_ViewDistance;
uniform mat4 u_InverseProjection;

vec3 blend(vec3 dst, vec4 src)
{
    return dst * (1.0 - src.a) + src.rgb * src.a;
}

float fog_factor_from_depth(float depth)
{
#if FOG_MODEL == 0
    return 0.0;
#else
    // Reconstruct view-space position; fog uses distance from the camera.
    vec4 clip = vec4(vs_TexCoord * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 view = u_InverseProjection * clip;
    float dist = length(view.xyz / view.w);

#if FOG_MODEL == 1
    return 1.0 - clamp((u_ViewDistance - dist) / (u_ViewDistance - 16.0), 0.0, 1.0);
#elif FOG_MODEL == 2
    float fogd = 2.0 / u_ViewDistance * dist;
    return 1.0 - clamp(exp2(fogd * fogd * -1.442695), 0.0, 1.0);
#else
    return 0.0;
#endif
#endif
}

vec4 fog_translucent(vec4 color, float depth)
{
#if FOG_MODEL
    float factor = fog_factor_from_depth(depth);
    color.rgb = mix(color.rgb, u_FogColor, factor);
    color.a *= 1.0 - factor;
#endif

    return color;
}

void main(void)
{
    vec4 colors[3];
    float depths[3];
    int order[3];

    colors[0] = vec4(texture(u_DiffuseColor, vs_TexCoord).rgb, 1.0);
    depths[0] = texture(u_DiffuseDepth, vs_TexCoord).r;
    order[0] = 0;

    colors[1] = vec4(0.0);
    depths[1] = 0.0;
    order[1] = 1;

    colors[2] = vec4(0.0);
    depths[2] = 0.0;
    order[2] = 2;

    int active_index = 1;
    vec4 alpha = texture(u_AlphaColor, vs_TexCoord);

    if(alpha.a > 0.0) {
        float alpha_depth = texture(u_AlphaDepth, vs_TexCoord).r;
        colors[active_index] = fog_translucent(alpha, alpha_depth);
        depths[active_index] = alpha_depth;
        order[active_index] = active_index;

        int jj = active_index;
        int ii = jj - 1;

        while(jj > 0 && depths[order[jj]] > depths[order[ii]]) {
            int tmp = order[ii];

            order[ii] = order[jj];
            order[jj] = tmp;

            jj = ii;
            ii = jj - 1;
        }

        active_index += 1;
    }

    vec4 fluid = texture(u_FluidColor, vs_TexCoord);

    if(fluid.a > 0.0) {
        float fluid_depth = texture(u_FluidDepth, vs_TexCoord).r;
        colors[active_index] = fog_translucent(fluid, fluid_depth);
        depths[active_index] = fluid_depth;
        order[active_index] = active_index;

        int jj = active_index;
        int ii = jj - 1;

        while(jj > 0 && depths[order[jj]] > depths[order[ii]]) {
            int tmp = order[ii];

            order[ii] = order[jj];
            order[jj] = tmp;

            jj = ii;
            ii = jj - 1;
        }

        active_index += 1;
    }

    vec3 accum = colors[order[0]].rgb;

    for(int i = 1; i < active_index; i += 1) {
        if(colors[order[i]].a > 0.0) {
            accum = blend(accum, colors[order[i]]);
        }
    }

    frag_Target = vec4(accum, 1.0);
}
