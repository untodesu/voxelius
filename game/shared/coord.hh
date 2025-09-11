#pragma once

#include "shared/const.hh"
#include "shared/types.hh"

namespace coord
{
constexpr chunk_pos to_chunk(const voxel_pos& vpos);
} // namespace coord

namespace coord
{
constexpr local_pos to_local(const voxel_pos& vpos);
constexpr local_pos to_local(const glm::fvec3& fvec);
constexpr local_pos to_local(std::size_t index);
} // namespace coord

namespace coord
{
constexpr voxel_pos to_voxel(const chunk_pos& cpos, const local_pos& lpos);
constexpr voxel_pos to_voxel(const chunk_pos& cpos, const glm::fvec3& fvec);
} // namespace coord

namespace coord
{
constexpr std::size_t to_index(const local_pos& lpos);
} // namespace coord

namespace coord
{
constexpr glm::fvec3 to_relative(const chunk_pos& pivot_cpos, const chunk_pos& cpos, const glm::fvec3& fvec);
constexpr glm::fvec3 to_relative(const chunk_pos& pivot_cpos, const glm::fvec3& pivot_fvec, const chunk_pos& cpos);
constexpr glm::fvec3 to_relative(const chunk_pos& pivot_cpos, const glm::fvec3& pivot_fvec, const chunk_pos& cpos, const glm::fvec3& fvec);
} // namespace coord

namespace coord
{
constexpr glm::fvec3 to_fvec3(const chunk_pos& cpos);
constexpr glm::fvec3 to_fvec3(const chunk_pos& cpos, const glm::fvec3& fpos);
} // namespace coord

inline constexpr chunk_pos coord::to_chunk(const voxel_pos& vpos)
{
    return chunk_pos {
        static_cast<chunk_pos::value_type>(vpos.x >> CHUNK_BITSHIFT),
        static_cast<chunk_pos::value_type>(vpos.y >> CHUNK_BITSHIFT),
        static_cast<chunk_pos::value_type>(vpos.z >> CHUNK_BITSHIFT),
    };
}

inline constexpr local_pos coord::to_local(const voxel_pos& vpos)
{
    return local_pos {
        static_cast<local_pos::value_type>(math::mod_signed<voxel_pos::value_type>(vpos.x, CHUNK_SIZE)),
        static_cast<local_pos::value_type>(math::mod_signed<voxel_pos::value_type>(vpos.y, CHUNK_SIZE)),
        static_cast<local_pos::value_type>(math::mod_signed<voxel_pos::value_type>(vpos.z, CHUNK_SIZE)),
    };
}

inline constexpr local_pos coord::to_local(const glm::fvec3& fvec)
{
    return local_pos {
        static_cast<local_pos::value_type>(fvec.x),
        static_cast<local_pos::value_type>(fvec.y),
        static_cast<local_pos::value_type>(fvec.z),
    };
}

inline constexpr local_pos coord::to_local(std::size_t index)
{
    return local_pos {
        static_cast<local_pos::value_type>((index % CHUNK_SIZE)),
        static_cast<local_pos::value_type>((index / CHUNK_SIZE) / CHUNK_SIZE),
        static_cast<local_pos::value_type>((index / CHUNK_SIZE) % CHUNK_SIZE),
    };
}

inline constexpr voxel_pos coord::to_voxel(const chunk_pos& cpos, const local_pos& lpos)
{
    return voxel_pos {
        lpos.x + (static_cast<voxel_pos::value_type>(cpos.x) << CHUNK_BITSHIFT),
        lpos.y + (static_cast<voxel_pos::value_type>(cpos.y) << CHUNK_BITSHIFT),
        lpos.z + (static_cast<voxel_pos::value_type>(cpos.z) << CHUNK_BITSHIFT),
    };
}

inline constexpr voxel_pos coord::to_voxel(const chunk_pos& cpos, const glm::fvec3& fvec)
{
    return voxel_pos {
        static_cast<voxel_pos::value_type>(fvec.x) + (static_cast<voxel_pos::value_type>(cpos.x) << CHUNK_BITSHIFT),
        static_cast<voxel_pos::value_type>(fvec.y) + (static_cast<voxel_pos::value_type>(cpos.y) << CHUNK_BITSHIFT),
        static_cast<voxel_pos::value_type>(fvec.z) + (static_cast<voxel_pos::value_type>(cpos.z) << CHUNK_BITSHIFT),
    };
}

inline constexpr std::size_t coord::to_index(const local_pos& lpos)
{
    return static_cast<std::size_t>((lpos.y * CHUNK_SIZE + lpos.z) * CHUNK_SIZE + lpos.x);
}

inline constexpr glm::fvec3 coord::to_relative(const chunk_pos& pivot_cpos, const chunk_pos& cpos, const glm::fvec3& fvec)
{
    return glm::fvec3 {
        static_cast<float>((cpos.x - pivot_cpos.x) << CHUNK_BITSHIFT) + fvec.x,
        static_cast<float>((cpos.y - pivot_cpos.y) << CHUNK_BITSHIFT) + fvec.y,
        static_cast<float>((cpos.z - pivot_cpos.z) << CHUNK_BITSHIFT) + fvec.z,
    };
}

inline constexpr glm::fvec3 coord::to_relative(const chunk_pos& pivot_cpos, const glm::fvec3& pivot_fvec, const chunk_pos& cpos)
{
    return glm::fvec3 {
        static_cast<float>((cpos.x - pivot_cpos.x) << CHUNK_BITSHIFT) - pivot_fvec.x,
        static_cast<float>((cpos.y - pivot_cpos.y) << CHUNK_BITSHIFT) - pivot_fvec.y,
        static_cast<float>((cpos.z - pivot_cpos.z) << CHUNK_BITSHIFT) - pivot_fvec.z,
    };
}

inline constexpr glm::fvec3 coord::to_relative(const chunk_pos& pivot_cpos, const glm::fvec3& pivot_fvec, const chunk_pos& cpos,
    const glm::fvec3& fvec)
{
    return glm::fvec3 {
        static_cast<float>((cpos.x - pivot_cpos.x) << CHUNK_BITSHIFT) + (fvec.x - pivot_fvec.x),
        static_cast<float>((cpos.y - pivot_cpos.y) << CHUNK_BITSHIFT) + (fvec.y - pivot_fvec.y),
        static_cast<float>((cpos.z - pivot_cpos.z) << CHUNK_BITSHIFT) + (fvec.z - pivot_fvec.z),
    };
}

inline constexpr glm::fvec3 coord::to_fvec3(const chunk_pos& cpos)
{
    return glm::fvec3 {
        static_cast<float>(cpos.x << CHUNK_BITSHIFT),
        static_cast<float>(cpos.y << CHUNK_BITSHIFT),
        static_cast<float>(cpos.z << CHUNK_BITSHIFT),
    };
}

inline constexpr glm::fvec3 coord::to_fvec3(const chunk_pos& cpos, const glm::fvec3& fpos)
{
    return glm::fvec3 {
        fpos.x + static_cast<float>(cpos.x << CHUNK_BITSHIFT),
        fpos.y + static_cast<float>(cpos.y << CHUNK_BITSHIFT),
        fpos.z + static_cast<float>(cpos.z << CHUNK_BITSHIFT),
    };
}
