#include "client/pch.hh"

#include "client/world/chunk_mesh.hh"

#include "client/world/chunk_vbo.hh"

static std::uint32_t s_slot_cap;
static std::vector<std::uint32_t> s_free_slots;

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

ChunkMesh_Part::~ChunkMesh_Part(void)
{
    if(count) {
        chunk_vbo::free(base_vertex, count);
    }
}

ChunkMesh_Part::ChunkMesh_Part(ChunkMesh_Part&& other)
    : vertices(std::move(other.vertices)), count(other.count), base_vertex(other.base_vertex)
{
    other.count = 0;
    other.base_vertex = 0;
}

ChunkMesh_Part& ChunkMesh_Part::operator=(ChunkMesh_Part&& other)
{
    if(this == &other) {
        return *this;
    }

    if(count) {
        chunk_vbo::free(base_vertex, count);
    }

    count = 0;
    base_vertex = 0;

    vertices = std::move(other.vertices);
    count = other.count;
    base_vertex = other.base_vertex;

    other.count = 0;
    other.base_vertex = 0;

    return *this;
}

ChunkMesh::ChunkMesh(void)
{
    if(s_free_slots.size()) {
        slot = s_free_slots.back();
        s_free_slots.pop_back();
    }
    else {
        slot = s_slot_cap;
        s_slot_cap += 1;
    }
}

ChunkMesh::~ChunkMesh(void)
{
    if(slot < std::numeric_limits<std::uint32_t>::max()) {
        s_free_slots.push_back(slot);
    }
}

ChunkMesh::ChunkMesh(ChunkMesh&& other)
    : slot(other.slot), opaque(std::move(other.opaque)), alpha(std::move(other.alpha)), fluid(std::move(other.fluid))
{
    other.slot = std::numeric_limits<std::uint32_t>::max();
}

ChunkMesh& ChunkMesh::operator=(ChunkMesh&& other)
{
    if(this == &other) {
        return *this;
    }

    slot = other.slot;
    opaque = std::move(other.opaque);
    alpha = std::move(other.alpha);
    fluid = std::move(other.fluid);

    other.slot = std::numeric_limits<std::uint32_t>::max();

    return *this;
}
