#ifndef CLIENT_CHUNK_QUAD_HH
#define CLIENT_CHUNK_QUAD_HH 1
#pragma once

#include "core/math/constexpr.hh"

#include "shared/world/voxel_registry.hh"

namespace world
{
// [0] XXXXXXXXYYYYYYYYZZZZZZZZWWWWHHHH
// [1] FFFFTTTTTTTTTTTAAAAA------------
using ChunkQuad = std::array<std::uint32_t, 2>;
} // namespace world

namespace world
{
constexpr inline static ChunkQuad make_chunk_quad(
    const glm::fvec3& position, const glm::fvec2& size, voxel_facing facing, std::size_t texture, std::size_t frames)
{
    ChunkQuad result = {};
    result[0] = 0x00000000;
    result[1] = 0x00000000;

    // [0] XXXXXXXXYYYYYYYYZZZZZZZZ--------
    result[0] |= (0x000000FFU & static_cast<std::uint32_t>(position.x * 16.0f)) << 24U;
    result[0] |= (0x000000FFU & static_cast<std::uint32_t>(position.y * 16.0f)) << 16U;
    result[0] |= (0x000000FFU & static_cast<std::uint32_t>(position.z * 16.0f)) << 8U;

    // [0] ------------------------WWWWHHHH
    result[0] |= (0x0000000FU & static_cast<std::uint32_t>(size.x * 16.0f - 1.0f)) << 4U;
    result[0] |= (0x0000000FU & static_cast<std::uint32_t>(size.y * 16.0f - 1.0f));

    // [1] FFFF----------------------------
    result[1] |= (0x0000000FU & static_cast<std::uint32_t>(facing)) << 28U;

    // [1] ----TTTTTTTTTTTAAAAA------------
    result[1] |= (0x000007FFU & static_cast<std::uint32_t>(texture)) << 17U;
    result[1] |= (0x0000001FU & static_cast<std::uint32_t>(frames)) << 12U;

    return result;
}
} // namespace world

#endif // CLIENT_CHUNK_QUAD_HH
