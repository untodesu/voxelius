#include "client/pch.hh"

#include "client/chunk_mesh.hh"

std::uint32_t ChunkMesh_Vertex::pack_position(const Eigen::Vector3f& position_16ths) noexcept
{
    auto biased_x = INT32_C(16) + static_cast<std::int32_t>(position_16ths.x());
    auto biased_y = INT32_C(16) + static_cast<std::int32_t>(position_16ths.y());
    auto biased_z = INT32_C(16) + static_cast<std::int32_t>(position_16ths.z());

    std::array<std::uint32_t, 3> packed_axes {};
    packed_axes[0] = static_cast<std::uint32_t>(std::clamp(biased_x, INT32_C(0), INT32_C(1023)));
    packed_axes[1] = static_cast<std::uint32_t>(std::clamp(biased_y, INT32_C(0), INT32_C(1023)));
    packed_axes[2] = static_cast<std::uint32_t>(std::clamp(biased_z, INT32_C(0), INT32_C(1023)));

    std::uint32_t result = 0;
    result |= packed_axes[0];
    result |= packed_axes[1] << 10U;
    result |= packed_axes[2] << 20U;
    return result;
}

Eigen::Vector3f ChunkMesh_Vertex::unpack_position(std::uint32_t position) noexcept
{
    auto x = static_cast<float>(static_cast<std::int32_t>(position & 0x3FFU) - INT32_C(16));
    auto y = static_cast<float>(static_cast<std::int32_t>((position >> 10U) & 0x3FFU) - INT32_C(16));
    auto z = static_cast<float>(static_cast<std::int32_t>((position >> 20U) & 0x3FFU) - INT32_C(16));
    return Eigen::Vector3f(x, y, z);
}
