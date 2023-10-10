// SPDX-License-Identifier: MPL-2.0
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#version 450 core
#pragma import common
#pragma import screenspace

layout(location = 0) out vec2 texcoord;
layout(location = 1) out vec2 pixcoord;
layout(location = 2) out vec4 colormod;

layout(std140, binding = 0) uniform Canvas_UBO {
    vec4 col_ul;
    vec4 col_ur;
    vec4 col_dl;
    vec4 col_dr;
    vec4 screen;
    vec4 glyph;
    vec4 rect;
};

void main(void)
{
    const vec2 oa = vec2(0.0, rect.w);
    const vec2 ob = vec2(rect.z, rect.w);
    const vec2 oc = vec2(rect.z, 0.0);
    const vec2 od = vec2(0.0, 0.0);
    const vec2 ox[6] = { oa, ob, oc, oc, od, oa };
    const vec4 cm[6] = {
        col_dl, col_dr, col_ur,
        col_ur, col_ul, col_dl,
    };

    const vec2 off = ox[gl_VertexID % 6];
    const vec2 ndc = 2.0 * screen.zw * (rect.xy + off) - 1.0;

    texcoord = screenspace_texcoord;
    pixcoord = texcoord * rect.zw;
    colormod = cm[gl_VertexID % 6];
    gl_Position = vec4(ndc.x, -ndc.y, 0.0, 1.0);
}
