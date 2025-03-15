#ifndef SHARED_TYPES_HH
#define SHARED_TYPES_HH 1
#pragma once

using item_id = std::uint32_t;
constexpr static item_id NULL_ITEM_ID = UINT32_C(0x00000000);
constexpr static item_id MAX_ITEM_ID = UINT32_C(0xFFFFFFFF);

using voxel_id = std::uint16_t;
constexpr static voxel_id NULL_VOXEL_ID = UINT16_C(0x0000);
constexpr static voxel_id MAX_VOXEL_ID = UINT16_C(0xFFFF);

using chunk_pos = glm::vec<3, std::int32_t>;
using local_pos = glm::vec<3, std::int16_t>;
using voxel_pos = glm::vec<3, std::int64_t>;

// A special 2D chunk coordinate used by world generation code
// to cache things like 2D noise and terrain heightmap for performance reasons
using worldgen_chunk_pos = glm::vec<2, chunk_pos::value_type>;

template<>
struct std::hash<chunk_pos> final {
    constexpr inline std::size_t operator()(const chunk_pos &cpos) const
    {
        std::size_t value = 0;
        value ^= cpos.x * 73856093;
        value ^= cpos.y * 19349663;
        value ^= cpos.z * 83492791;
        return value;
    }
};

template<>
struct std::hash<worldgen_chunk_pos> final {
    constexpr inline std::size_t operator()(const worldgen_chunk_pos &cwpos) const
    {
        std::size_t value = 0;
        value ^= cwpos.x * 73856093;
        value ^= cwpos.y * 19349663;
        return value;
    }
};

#endif /* SHARED_TYPES_HH */
