// SPDX-License-Identifier: MPL-2.0
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#version 450 core
#pragma import common

layout(location = 0) in vec2 texcoord;

layout(location = 0) out vec4 target;

layout(binding = 0) uniform sampler2D solid_albedo;
layout(binding = 1) uniform sampler2D solid_normal;

void main(void)
{
    const vec4 albedo = texture(solid_albedo, texcoord);
    const vec3 normal = texture(solid_normal, texcoord).xyz;
    target = vec4(0.0, 0.0, 0.0, 1.0);
    target.rgb += albedo.rgb;
}