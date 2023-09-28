// SPDX-License-Identifier: MPL-2.0
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef C818F02B_A5DD_4849_955F_5D6CEAC97830
#define C818F02B_A5DD_4849_955F_5D6CEAC97830
#include <client/glxx/fwd.hh>
#include <shared/types.hh>

constexpr static const uint32_t VOXEL_VERTEX_VBO_BINDING = 0;

struct VoxelVertex final {
    // vvdat_i[0]: 3x10-bit vertex position
    // vvdat_i[1]: 3x10-bit vertex normal
    // vvdat_i[2]: 2x16-bit texture coords
    // vvdat_i[3]: 1x16-bit toffset, 1x16-bit tframes
    uint32_t vvdat_i[4] {};

    VoxelVertex() = default;
    VoxelVertex(const vec3f_t &position, const vec3f_t &normal, const vec2f_t &uv, uint16_t toffset, uint16_t tframes);
    static void setup(glxx::VertexArray &vao);
    static void init();
};

#endif/* C818F02B_A5DD_4849_955F_5D6CEAC97830 */