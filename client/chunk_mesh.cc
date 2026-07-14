#include "client/pch.hh"

#include "client/chunk_mesh.hh"

static std::uint32_t pack_10_10_10(const Eigen::Vector3f& value_16ths, std::int32_t bias)
{
    auto bx = bias + static_cast<std::int32_t>(std::lround(value_16ths.x()));
    auto by = bias + static_cast<std::int32_t>(std::lround(value_16ths.y()));
    auto bz = bias + static_cast<std::int32_t>(std::lround(value_16ths.z()));

    auto px = static_cast<std::uint32_t>(std::clamp(bx, INT32_C(0), INT32_C(1023)));
    auto py = static_cast<std::uint32_t>(std::clamp(by, INT32_C(0), INT32_C(1023)));
    auto pz = static_cast<std::uint32_t>(std::clamp(bz, INT32_C(0), INT32_C(1023)));

    return px | (py << 10U) | (pz << 20U);
}

static std::uint32_t unorm8(float value)
{
    return static_cast<std::uint32_t>(std::lround(std::clamp(value, 0.0f, 1.0f) * 255.0f)) & 0xFFU;
}

ChunkMesh_Part::~ChunkMesh_Part(void)
{
    if(vbo) {
        // The ChunkMesh_Part structure is meant to be a part
        // of the ChunkMesh component within the EnTT registry...
        // When the registry is cleaned or a chunk is removed, components
        // are expected to be safely disposed of so we need a destructor
        glDeleteBuffers(1, &vbo);
    }
}

std::uint32_t ChunkMesh_Quad::pack_position(const Eigen::Vector3f& position_16ths)
{
    return pack_10_10_10(position_16ths, INT32_C(16));
}

std::uint32_t ChunkMesh_Quad::pack_offset(const Eigen::Vector3f& offset_16ths)
{
    return pack_10_10_10(offset_16ths, INT32_C(512));
}

std::uint32_t ChunkMesh_Quad::pack_uv(const Eigen::Vector2f& c0, const Eigen::Vector2f& c2)
{
    std::uint32_t result = 0;
    result |= unorm8(c0.x()) << 0U;
    result |= unorm8(c0.y()) << 8U;
    result |= unorm8(c2.x()) << 16U;
    result |= unorm8(c2.y()) << 24U;
    return result;
}

std::uint32_t ChunkMesh_Quad::pack_texture(std::uint32_t texture_index, std::uint32_t frame_offset, std::uint32_t tint_index)
{
    std::uint32_t result = texture_index & 0xFFFFU;
    result |= (frame_offset & 0xFFU) << 16U;
    result |= (tint_index & 0xFFU) << 24U;
    return result;
}

std::uint32_t ChunkMesh_Quad::pack_extras(const std::array<std::uint32_t, 4>& ao)
{
    std::uint32_t result = 0;
    result |= (ao[0] & 0x03U);
    result |= (ao[1] & 0x03U) << 2U;
    result |= (ao[2] & 0x03U) << 4U;
    result |= (ao[3] & 0x03U) << 6U;
    return result;
}
