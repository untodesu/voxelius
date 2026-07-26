#include "client/pch.hh"

#include "client/world/chunk_mesh.hh"

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

static std::uint32_t pack_unorm8(float value)
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
        vbo = 0;
    }
}

ChunkMesh_Part::ChunkMesh_Part(ChunkMesh_Part&& other) : vertices(std::move(other.vertices)), count(other.count), vbo(other.vbo)
{
    other.count = 0;
    other.vbo = 0;
}

ChunkMesh_Part& ChunkMesh_Part::operator=(ChunkMesh_Part&& other)
{
    if(this != &other) {
        if(vbo) {
            glDeleteBuffers(1, &vbo);
        }

        vertices = std::move(other.vertices);
        count = other.count;
        vbo = other.vbo;

        other.count = 0;
        other.vbo = 0;
    }

    return *this;
}

std::uint32_t ChunkMesh_Vertex::pack_position(const Eigen::Vector3f& position_16ths)
{
    return pack_10_10_10(position_16ths, INT32_C(16));
}

std::uint32_t ChunkMesh_Vertex::pack_uv(const Eigen::Vector2f& uv)
{
    std::uint32_t result = 0;
    result |= pack_unorm8(uv.x());
    result |= pack_unorm8(uv.y()) << 8U;
    return result;
}

std::uint32_t ChunkMesh_Vertex::pack_texture(std::uint32_t texture_index, std::uint32_t frame_offset, std::uint32_t tint_index)
{
    std::uint32_t result = texture_index & 0xFFFFU;
    result |= (frame_offset & 0xFFU) << 16U;
    result |= (tint_index & 0xFFU) << 24U;
    return result;
}

std::uint32_t ChunkMesh_Vertex::pack_extras(std::uint32_t ao, float shade, bool animated)
{
    std::uint32_t result = 0;
    result |= ao & 0x03U;
    result |= pack_unorm8(shade) << 8U;
    result |= animated ? ANIMATED_BIT : 0U;
    return result;
}
