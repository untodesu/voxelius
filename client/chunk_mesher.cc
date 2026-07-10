#include "client/pch.hh"

#include "client/chunk_mesher.hh"

#include "core/threading.hh"

#include "shared/utils/coord.hh"

#include "shared/block_registry.hh"
#include "shared/block_storage.hh"
#include "shared/chunk.hh"
#include "shared/constant.hh"
#include "shared/coord.hh"
#include "shared/globals.hh"
#include "shared/world.hh"

#include "client/block_models.hh"
#include "client/chunk_mesh.hh"

constexpr static std::array ALL_FACES = {
    BLOCK_FACE_NORTH,
    BLOCK_FACE_SOUTH,
    BLOCK_FACE_EAST,
    BLOCK_FACE_WEST,
    BLOCK_FACE_TOP,
    BLOCK_FACE_BOTTOM,
};

// Fake-lighting multiplier per dominant normal axis, ported from the old
// chunk_quad.vert.glsl `shades[]` table (indexed there by facing, here by normal)
static float shade_factor(std::uint32_t packed_normal) noexcept
{
    auto unpack10 = [](std::uint32_t bits) noexcept -> float {
        auto signed_bits = static_cast<std::int32_t>(bits << 22) >> 22; // sign-extend a 10-bit field
        return static_cast<float>(signed_bits) / 511.0f;
    };

    auto nx = unpack10(packed_normal & 0x3FFU);
    auto ny = unpack10((packed_normal >> 10U) & 0x3FFU);
    auto nz = unpack10((packed_normal >> 20U) & 0x3FFU);

    auto ax = std::abs(nx);
    auto ay = std::abs(ny);
    auto az = std::abs(nz);

    if((ay >= ax) && (ay >= az)) {
        return (ny >= 0.0f) ? 1.0f : 0.4f; // top : bottom
    }

    if(ax >= az) {
        return 0.6f; // east/west
    }

    return 0.8f; // north/south
}

class MeshingTask final : public Task {
public:
    explicit MeshingTask(entt::entity entity, const chunk_pos& cpos);
    virtual ~MeshingTask(void) noexcept override = default;
    virtual void process(void) noexcept override;
    virtual void finalize(void) noexcept override;

private:
    bool is_culled(const local_pos& lpos, block_face face, block_render self_render) const noexcept;
    void emit_quads(std::vector<ChunkMesh_Vertex>& out, std::span<const BakedBlockModel_Quad> quads, const local_pos& lpos) const noexcept;
    void mesh_block(const local_pos& lpos, block_id_type id) noexcept;

    std::array<BlockStorage, 7> m_cache;
    std::vector<ChunkMesh_Vertex> m_opaque;
    std::vector<ChunkMesh_Vertex> m_alpha;
    entt::entity m_entity;
    chunk_pos m_cpos;
};

constexpr static block_face opposite_face(block_face face) noexcept
{
    switch(face) {
        case BLOCK_FACE_NORTH:
            return BLOCK_FACE_SOUTH;

        case BLOCK_FACE_SOUTH:
            return BLOCK_FACE_NORTH;

        case BLOCK_FACE_EAST:
            return BLOCK_FACE_WEST;

        case BLOCK_FACE_WEST:
            return BLOCK_FACE_EAST;

        case BLOCK_FACE_TOP:
            return BLOCK_FACE_BOTTOM;

        case BLOCK_FACE_BOTTOM:
            return BLOCK_FACE_TOP;
    }

    return BLOCK_FACE_NORTH;
}

static chunk_pos face_delta(block_face face) noexcept
{
    switch(face) {
        case BLOCK_FACE_NORTH:
            return -chunk_pos::UnitZ();

        case BLOCK_FACE_SOUTH:
            return chunk_pos::UnitZ();

        case BLOCK_FACE_EAST:
            return chunk_pos::UnitX();

        case BLOCK_FACE_WEST:
            return -chunk_pos::UnitX();

        case BLOCK_FACE_TOP:
            return chunk_pos::UnitY();

        case BLOCK_FACE_BOTTOM:
            return -chunk_pos::UnitY();
    }

    return chunk_pos::Zero();
}

static bool is_neighbour(const local_pos& lpos) noexcept
{
    auto result = false;
    result = result || lpos.x() < 0 || lpos.x() >= constant::CHUNK_SIZE;
    result = result || lpos.y() < 0 || lpos.y() >= constant::CHUNK_SIZE;
    result = result || lpos.z() < 0 || lpos.z() >= constant::CHUNK_SIZE;
    return result;
}

static void mark_dirty(entt::entity entity) noexcept
{
    world::chunk_entities.emplace_or_replace<ChunkMeshDirtyComponent>(entity);
}

static void mark_dirty(const chunk_pos& cpos) noexcept
{
    if(auto chunk = world::find_chunk(cpos)) {
        mark_dirty(chunk->entity());
    }
}

MeshingTask::MeshingTask(entt::entity entity, const chunk_pos& cpos) : m_entity(entity), m_cpos(cpos)
{
    if(auto chunk = world::find_chunk(cpos)) {
        m_cache[0] = chunk->blocks();
    }

    for(std::size_t i = 0; i < ALL_FACES.size(); ++i) {
        auto neighbor_cpos = cpos + face_delta(ALL_FACES[i]);

        if(auto chunk = world::find_chunk(neighbor_cpos)) {
            m_cache[1 + i] = chunk->blocks();
        }
    }
}

void MeshingTask::process(void) noexcept
{
    for(std::size_t i = 0; i < constant::CHUNK_VOLUME; ++i) {
        if(status.load(std::memory_order_relaxed) == task_status::CANCELLED) {
            m_opaque.clear();
            m_alpha.clear();
            return;
        }

        mesh_block(utils::to_local(i), m_cache[0].get(i));
    }
}

void MeshingTask::finalize(void) noexcept
{
    if(world::chunk_entities.valid(m_entity)) {
        if(m_opaque.empty() && m_alpha.empty()) {
            world::chunk_entities.remove<ChunkMeshComponent>(m_entity);
            return;
        }

        auto& component = world::chunk_entities.emplace_or_replace<ChunkMeshComponent>(m_entity);
        component.opaque_vertices = std::move(m_opaque);
        component.blend_vertices = std::move(m_alpha);

        // GPU buffer upload belongs to the rendering pass, not yet wired up here
    }
}

bool MeshingTask::is_culled(const local_pos& lpos, block_face face, block_render self_render) const noexcept
{
    auto neighbour_lpos = lpos + face_delta(face);
    auto query_lpos = neighbour_lpos.eval();
    auto storage = &m_cache[0];

    if(is_neighbour(neighbour_lpos)) {
        storage = &m_cache[1 + static_cast<std::size_t>(face)];
        query_lpos = utils::wrap_local(neighbour_lpos);
    }

    auto neighbour_id = storage->get(query_lpos);

    if(neighbour_id == BLOCK_ID_NULL) {
        return false;
    }

    auto neighbour_baked = block_models::find(neighbour_id);
    auto neighbour_def = block_registry::find_definition(neighbour_id);

    if(neighbour_baked == nullptr || neighbour_def == nullptr) {
        return false;
    }

    if(neighbour_def->render != self_render) {
        // Differing render modes shouldn't cull
        // each other; this way stuff like glass
        // panes don't cull ground beneath them
        return false;
    }

    return neighbour_baked->fully_covered[opposite_face(face)];
}

void MeshingTask::emit_quads(std::vector<ChunkMesh_Vertex>& out, std::span<const BakedBlockModel_Quad> quads,
    const local_pos& lpos) const noexcept
{
    auto block_origin = lpos.cast<float>() * 16.0f;

    for(auto& quad : quads) {
        auto shade = quad.shade ? shade_factor(quad.packed_normal) : 1.0f;

        for(std::size_t i = 0; i < quad.positions.size(); ++i) {
            ChunkMesh_Vertex vertex {};
            vertex.position = ChunkMesh_Vertex::pack_position(block_origin + quad.positions[i] * 16.0f);
            vertex.normal = quad.packed_normal;
            vertex.uv[0] = quad.uvs[i].x();
            vertex.uv[1] = quad.uvs[i].y();
            vertex.frame_base = static_cast<std::uint32_t>(quad.frame_base);
            vertex.frame_count = static_cast<std::uint32_t>(quad.frame_count);
            vertex.tint_index = quad.tint_index;
            vertex.shade = shade;
            out.push_back(vertex);
        }
    }
}

void MeshingTask::mesh_block(const local_pos& lpos, block_id_type id) noexcept
{
    if(id == BLOCK_ID_NULL) {
        return;
    }

    auto baked = block_models::find(id);
    auto def = block_registry::find_definition(id);

    if(baked == nullptr || def == nullptr) {
        return;
    }

    std::array<bool, 6> culled {};
    auto fully_enclosed = true;

    for(auto face : ALL_FACES) {
        culled[face] = is_culled(lpos, face, def->render);
        fully_enclosed = fully_enclosed && culled[face];
    }

    if(def->render == BLOCK_RENDER_ALPHA) {
        if(!fully_enclosed) {
            emit_quads(m_alpha, baked->unculled_quads, lpos);
        }

        for(auto face : ALL_FACES) {
            if(culled[face])
                continue;
            emit_quads(m_alpha, baked->face_quads[face], lpos);
        }
    }
    else {
        if(!fully_enclosed) {
            emit_quads(m_opaque, baked->unculled_quads, lpos);
        }

        for(auto face : ALL_FACES) {
            if(culled[face])
                continue;
            emit_quads(m_opaque, baked->face_quads[face], lpos);
        }
    }
}

static void mark_neighbors_dirty(const chunk_pos& cpos) noexcept
{
    for(auto face : ALL_FACES) {
        mark_dirty(cpos + face_delta(face));
    }
}

static void on_chunk_create(const ChunkCreateEvent& event) noexcept
{
    auto chunk = event.chunk();
    mark_dirty(chunk->entity());
    mark_neighbors_dirty(event.pos());
}

static void on_chunk_update(const ChunkUpdateEvent& event) noexcept
{
    auto chunk = event.chunk();
    mark_dirty(chunk->entity());
    mark_neighbors_dirty(event.pos());
}

static void on_chunk_remove(const ChunkRemoveEvent& event) noexcept
{
    auto chunk = event.chunk();
    auto entity = chunk->entity();

    if(world::chunk_entities.valid(entity)) {
        world::chunk_entities.remove<ChunkMeshDirtyComponent>(entity);
        world::chunk_entities.remove<ChunkMeshComponent>(entity);
    }
}

static void on_block_update(const BlockUpdateEvent& event) noexcept
{
    auto chunk = event.chunk();
    mark_dirty(chunk->entity());
    mark_neighbors_dirty(event.cpos());
}

void chunk_mesher::init(void)
{
    globals::dispatcher.sink<ChunkCreateEvent>().connect<&on_chunk_create>();
    globals::dispatcher.sink<ChunkUpdateEvent>().connect<&on_chunk_update>();
    globals::dispatcher.sink<ChunkRemoveEvent>().connect<&on_chunk_remove>();
    globals::dispatcher.sink<BlockUpdateEvent>().connect<&on_block_update>();
}

void chunk_mesher::update(void)
{
    auto group = world::chunk_entities.group<ChunkMeshDirtyComponent>(entt::get<ChunkComponent>);

    for(const auto [entity, chunk] : group.each()) {
        world::chunk_entities.remove<ChunkMeshDirtyComponent>(entity);
        threading::submit<MeshingTask>(entity, chunk.position);
    }
}
