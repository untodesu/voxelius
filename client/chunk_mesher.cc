#include "client/pch.hh"

#include "client/chunk_mesher.hh"

#include "core/threading.hh"

#include "shared/block_registry.hh"
#include "shared/block_storage.hh"
#include "shared/chunk.hh"
#include "shared/constant.hh"
#include "shared/coord.hh"
#include "shared/globals.hh"
#include "shared/utils/coord.hh"
#include "shared/world.hh"

#include "client/block_models.hh"
#include "client/chunk_mesh.hh"

constexpr static std::size_t THROTTLE_COUNT = 32;

constexpr static std::array ALL_FACES = {
    BLOCK_FACE_NORTH,
    BLOCK_FACE_SOUTH,
    BLOCK_FACE_EAST,
    BLOCK_FACE_WEST,
    BLOCK_FACE_TOP,
    BLOCK_FACE_BOTTOM,
};

class MeshingTask final : public Task {
public:
    explicit MeshingTask(entt::entity entity, const chunk_pos& cpos);
    virtual ~MeshingTask(void) override = default;
    virtual void process(void) override;
    virtual void finalize(void) override;

private:
    bool is_culled(const local_pos& lpos, block_face face, block_render self_render) const;
    void emit_quads(std::vector<ChunkMesh_Quad>& out, std::span<const BakedBlockModel_Quad> quads, const local_pos& lpos,
        std::uint64_t entropy) const;
    void mesh_block(const local_pos& lpos, block_id_type id);

    std::array<BlockStorage, 7> m_cache;
    std::vector<ChunkMesh_Quad> m_opaque;
    std::vector<ChunkMesh_Quad> m_alpha;
    entt::entity m_entity;
    chunk_pos m_cpos;
};

constexpr static block_face opposite_face(block_face face)
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

static chunk_pos face_delta(block_face face)
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

static bool is_neighbour(const local_pos& lpos)
{
    auto result = false;
    result = result || lpos.x() < 0 || lpos.x() >= constant::CHUNK_SIZE;
    result = result || lpos.y() < 0 || lpos.y() >= constant::CHUNK_SIZE;
    result = result || lpos.z() < 0 || lpos.z() >= constant::CHUNK_SIZE;
    return result;
}

static void mark_dirty(entt::entity entity)
{
    world::chunk_entities.emplace_or_replace<ChunkMesh_DirtyMarker>(entity);
}

static void mark_dirty(const chunk_pos& cpos)
{
    if(auto chunk = world::find_chunk(cpos)) {
        mark_dirty(chunk->entity());
    }
}

static void sync_part(ChunkMesh_Part& part)
{
    if(part.quads.empty()) {
        if(part.vbo) {
            glDeleteBuffers(1, &part.vbo);
        }

        part.count = 0;
        part.vbo = 0;
        return;
    }

    if(part.vbo == 0)
        glGenBuffers(1, &part.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, part.vbo);
    glBufferData(GL_ARRAY_BUFFER, std::span(part.quads).size_bytes(), part.quads.data(), GL_STATIC_DRAW);
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

void MeshingTask::process(void)
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

void MeshingTask::finalize(void)
{
    if(world::chunk_entities.valid(m_entity)) {
        if(m_opaque.empty() && m_alpha.empty()) {
            world::chunk_entities.remove<ChunkMesh>(m_entity);
            return;
        }

        auto& component = world::chunk_entities.get_or_emplace<ChunkMesh>(m_entity);

        component.opaque.quads = std::move(m_opaque);
        component.opaque.count = static_cast<std::uint32_t>(component.opaque.quads.size());
        sync_part(component.opaque);

        component.alpha.quads = std::move(m_alpha);
        component.alpha.count = static_cast<std::uint32_t>(component.alpha.quads.size());
        sync_part(component.alpha);

        // Opaque quads won't get depth-sorted
        // at runtime so there's no point in keeping
        // them persistent in the system memory
        component.opaque.quads.clear();
        component.opaque.quads.shrink_to_fit();

        world::chunk_entities.patch<ChunkMesh>(m_entity);
    }
}

bool MeshingTask::is_culled(const local_pos& lpos, block_face face, block_render self_render) const
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

void MeshingTask::emit_quads(std::vector<ChunkMesh_Quad>& out, std::span<const BakedBlockModel_Quad> quads, const local_pos& lpos,
    std::uint64_t entropy) const
{
    auto block_origin = lpos.cast<float>() * 16.0f;

    for(const auto& quad : quads) {
        std::uint32_t frame_offset = 0;

        if(quad.frame_count > 0 && !quad.animated) {
            frame_offset = static_cast<std::uint32_t>(entropy % quad.frame_count);
        }

        Eigen::Vector3f p0 = block_origin + quad.positions[0] * 16.0f;
        Eigen::Vector3f p1 = block_origin + quad.positions[1] * 16.0f;
        Eigen::Vector3f p3 = block_origin + quad.positions[3] * 16.0f;

        const auto& c0 = quad.uvs[0];
        const auto& c2 = quad.uvs[2];

        auto dist_c0 = std::abs(quad.uvs[1].x() - c0.x());
        auto dist_c2 = std::abs(quad.uvs[1].x() - c2.x());
        auto uv_orient_flag = dist_c0 < dist_c2;

        ChunkMesh_Quad out_quad {};
        out_quad.data_origin = ChunkMesh_Quad::pack_position(p0);
        out_quad.data_edge_u = ChunkMesh_Quad::pack_offset(p1 - p0);
        out_quad.data_edge_v = ChunkMesh_Quad::pack_offset(p3 - p0);

        if(quad.shade) {
            out_quad.data_origin |= ChunkMesh_Quad::SHADE_BIT;
        }

        if(uv_orient_flag) {
            out_quad.data_origin |= ChunkMesh_Quad::UV_ORIENT_BIT;
        }

        if(quad.animated) {
            out_quad.data_edge_u |= ChunkMesh_Quad::ANIMATED_BIT;
        }

        out_quad.data_uv = ChunkMesh_Quad::pack_uv(c0, c2);
        out_quad.data_texture = ChunkMesh_Quad::pack_texture(quad.texture_index, frame_offset, quad.tint_index);

        out.push_back(out_quad);
    }
}

void MeshingTask::mesh_block(const local_pos& lpos, block_id_type id)
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

    std::uint64_t entropy;

    auto bpos = utils::to_block(m_cpos, lpos);
    entropy = static_cast<std::uint64_t>(bpos.x()) * UINT64_C(0x9E3779B97F4A7C15);
    entropy ^= static_cast<std::uint64_t>(bpos.y()) * UINT64_C(0xC2B2AE3D27D4EB4F);
    entropy ^= static_cast<std::uint64_t>(bpos.z()) * UINT64_C(0x165667B19E3779F9);

    // Just a simple-ish hash for a single 64-bit value; before that
    // we were using CRC64 which is kind of an overkill for a mesher
    // https://zimbry.blogspot.com/2011/09/better-bit-mixing-improving-on.html
    entropy ^= entropy >> 33;
    entropy *= UINT64_C(0xFF51AFD7ED558CCD);
    entropy ^= entropy >> 33;
    entropy *= UINT64_C(0xC4CEB9FE1A85EC53);
    entropy ^= entropy >> 33;

    if(def->render == BLOCK_RENDER_ALPHA) {
        if(!fully_enclosed) {
            emit_quads(m_alpha, baked->unculled_quads, lpos, entropy);
        }

        for(auto face : ALL_FACES) {
            if(culled[face])
                continue;
            emit_quads(m_alpha, baked->face_quads[face], lpos, entropy);
        }
    }
    else {
        if(!fully_enclosed) {
            emit_quads(m_opaque, baked->unculled_quads, lpos, entropy);
        }

        for(auto face : ALL_FACES) {
            if(culled[face])
                continue;
            emit_quads(m_opaque, baked->face_quads[face], lpos, entropy);
        }
    }
}

static void mark_neighbors_dirty(const chunk_pos& cpos)
{
    for(auto face : ALL_FACES) {
        mark_dirty(cpos + face_delta(face));
    }
}

static void mark_border_neighbors_dirty(const chunk_pos& cpos, const local_pos& lpos)
{
    for(auto face : ALL_FACES) {
        if(is_neighbour((lpos + face_delta(face)).eval())) {
            mark_dirty(cpos + face_delta(face));
        }
    }
}

static void on_chunk_create(const ChunkCreateEvent& event)
{
    auto chunk = event.chunk();
    mark_dirty(chunk->entity());
    mark_neighbors_dirty(event.pos());
}

static void on_chunk_update(const ChunkUpdateEvent& event)
{
    auto chunk = event.chunk();
    mark_dirty(chunk->entity());
    mark_neighbors_dirty(event.pos());
}

static void on_chunk_remove(const ChunkRemoveEvent& event)
{
    auto chunk = event.chunk();
    auto entity = chunk->entity();

    if(world::chunk_entities.valid(entity)) {
        world::chunk_entities.remove<ChunkMesh_DirtyMarker>(entity);
        world::chunk_entities.remove<ChunkMesh>(entity);
    }
}

static void on_block_update(const BlockUpdateEvent& event)
{
    auto chunk = event.chunk();
    mark_dirty(chunk->entity());
    mark_border_neighbors_dirty(event.cpos(), event.lpos());
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
    auto group = world::chunk_entities.group<ChunkMesh_DirtyMarker>(entt::get<Chunk_Component>);
    auto count = 0;

    for(const auto [entity, chunk] : group.each()) {
        world::chunk_entities.remove<ChunkMesh_DirtyMarker>(entity);
        threading::submit<MeshingTask>(entity, chunk.position);

        count += 1;

        if(count >= THROTTLE_COUNT) {
            break;
        }
    }
}
