#version 330 core

in vec2 vs_TexCoord;

out vec4 frag_Target;

uniform sampler2D u_DiffuseColor;
uniform sampler2D u_DiffuseDepth;

uniform sampler2D u_AlphaColor;
uniform sampler2D u_AlphaDepth;

uniform sampler2D u_FluidColor;
uniform sampler2D u_FluidDepth;

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
        colors[active_index] = alpha;
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
        colors[active_index] = fluid;
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

    for(int i = 1; i < active_index; ++i) {
        if(colors[order[i]].a > 0.0) {
            accum = accum * (1.0 - colors[order[i]].a) + colors[order[i]].rgb;
        }
    }

    frag_Target = vec4(accum, 1.0);
}
