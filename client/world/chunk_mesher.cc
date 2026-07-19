#include "client/pch.hh"

#include "client/world/chunk_mesher.hh"

#include "core/threading.hh"
#include "core/utils/modulo.hh"

#include "shared/constant.hh"
#include "shared/coord.hh"
#include "shared/globals.hh"
#include "shared/utils/coord.hh"
#include "shared/world/block_registry.hh"
#include "shared/world/block_storage.hh"
#include "shared/world/chunk.hh"
#include "shared/world/fluid_registry.hh"
#include "shared/world/world.hh"

#include "client/world/block_atlas.hh"
#include "client/world/block_models.hh"
#include "client/world/chunk_mesh.hh"
#include "client/world/fluid_cache.hh"

constexpr static std::size_t THROTTLE_COUNT = 32;

constexpr static std::array ALL_FACES = {
    BLOCK_FACE_NORTH,
    BLOCK_FACE_SOUTH,
    BLOCK_FACE_EAST,
    BLOCK_FACE_WEST,
    BLOCK_FACE_TOP,
    BLOCK_FACE_BOTTOM,
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

static ChunkPos face_delta(block_face face)
{
    switch(face) {
        case BLOCK_FACE_NORTH:
            return -ChunkPos::UnitZ();

        case BLOCK_FACE_SOUTH:
            return ChunkPos::UnitZ();

        case BLOCK_FACE_EAST:
            return ChunkPos::UnitX();

        case BLOCK_FACE_WEST:
            return -ChunkPos::UnitX();

        case BLOCK_FACE_TOP:
            return ChunkPos::UnitY();

        case BLOCK_FACE_BOTTOM:
            return -ChunkPos::UnitY();
    }

    return ChunkPos::Zero();
}

class BlockCache final {
public:
    constexpr static std::int16_t PADDING = 2;
    constexpr static std::int16_t CHUNK_SIZE_I16 = static_cast<std::int16_t>(constant::CHUNK_SIZE);

    constexpr static std::size_t SIZE = constant::CHUNK_SIZE + 2 * PADDING;
    constexpr static std::size_t VOLUME = SIZE * SIZE * SIZE;

    void init(const ChunkPos& cpos);

    block_id_type get(const LocalPos& lpos) const;

private:
    std::array<block_id_type, VOLUME> m_blocks;
};

void BlockCache::init(const ChunkPos& cpos)
{
    std::shared_ptr<const Chunk> chunks[3][3][3] = {};

    for(ChunkPos::value_type dz = -1; dz <= 1; dz += 1) {
        for(ChunkPos::value_type dy = -1; dy <= 1; dy += 1) {
            for(ChunkPos::value_type dx = -1; dx <= 1; dx += 1) {
                auto delta = ChunkPos(dx, dy, dz);
                auto query_pos = cpos + delta;

                if(auto chunk = world::find_chunk(query_pos)) {
                    chunks[dx + 1][dy + 1][dz + 1] = chunk;
                }
            }
        }
    }

    std::size_t index = 0;

    for(LocalPos::value_type lz = -PADDING; lz < CHUNK_SIZE_I16 + PADDING; lz += 1) {
        for(LocalPos::value_type ly = -PADDING; ly < CHUNK_SIZE_I16 + PADDING; ly += 1) {
            for(LocalPos::value_type lx = -PADDING; lx < CHUNK_SIZE_I16 + PADDING; lx += 1) {
                std::size_t index_x = 0;
                std::size_t index_y = 0;
                std::size_t index_z = 0;

                if(lx >= CHUNK_SIZE_I16) {
                    index_x = 2;
                }
                else if(lx >= 0) {
                    index_x = 1;
                }

                if(ly >= CHUNK_SIZE_I16) {
                    index_y = 2;
                }
                else if(ly >= 0) {
                    index_y = 1;
                }

                if(lz >= CHUNK_SIZE_I16) {
                    index_z = 2;
                }
                else if(lz >= 0) {
                    index_z = 1;
                }

                auto& chunk = chunks[index_x][index_y][index_z];

                if(chunk == nullptr) {
                    m_blocks[index] = BLOCK_ID_NULL;
                }
                else {
                    auto query_lx = utils::mod_signed<LocalPos::value_type>(lx, constant::CHUNK_SIZE);
                    auto query_ly = utils::mod_signed<LocalPos::value_type>(ly, constant::CHUNK_SIZE);
                    auto query_lz = utils::mod_signed<LocalPos::value_type>(lz, constant::CHUNK_SIZE);
                    m_blocks[index] = chunk->get_block(LocalPos(query_lx, query_ly, query_lz));
                }

                index += 1;
            }
        }
    }
}

block_id_type BlockCache::get(const LocalPos& lpos) const
{
    std::size_t index = 0;
    index += static_cast<std::size_t>(lpos.x() + PADDING);
    index += static_cast<std::size_t>(lpos.y() + PADDING) * SIZE;
    index += static_cast<std::size_t>(lpos.z() + PADDING) * SIZE * SIZE;

    if(index >= VOLUME)
        return BLOCK_ID_NULL;
    return m_blocks[index];
}

static emhash8::HashMap<ChunkPos, std::nullptr_t> s_pending;

class MeshingTask final : public Task {
public:
    explicit MeshingTask(entt::entity entity, const ChunkPos& cpos);
    virtual ~MeshingTask(void) override = default;
    virtual void process(void) override;
    virtual void finalize(void) override;

private:
    std::uint32_t calculate_ao(const LocalPos& lpos, block_face face, const Eigen::Vector3f& vertex) const;

    bool is_culled_block(const LocalPos& lpos, block_face face, block_render self_render) const;
    bool is_culled_fluid(const LocalPos& lpos, block_face face, fluid_id_type fluid_id) const;
    bool is_occluder(const LocalPos& lpos, block_face exposed_face) const;

    void emit_block_quads(std::vector<ChunkMesh_Quad>& out, std::span<const BakedBlockModel_Quad> quads, const LocalPos& lpos,
        std::uint64_t entropy, std::optional<block_face> face) const;
    void emit_fluid_quad(std::vector<ChunkMesh_Quad>& out, const LocalPos& lpos, const Eigen::Vector3f& min, const Eigen::Vector3f& max,
        block_face face, const AtlasStrip* strip, unsigned tint_index) const;

    void mesh_fluid(const LocalPos& lpos, const BlockDefinition& def);
    void mesh_block(const LocalPos& lpos, block_id_type id);

    BlockCache m_cache;
    std::vector<ChunkMesh_Quad> m_opaque;
    std::vector<ChunkMesh_Quad> m_alpha;
    std::vector<ChunkMesh_Quad> m_fluid;
    entt::entity m_entity;
    ChunkPos m_cpos;
};

static void sync_part(ChunkMesh_Part& part)
{
    if(part.quads.empty()) {
        if(part.vbo) {
            glDeleteBuffers(1, &part.vbo);
            part.vbo = 0;
        }

        part.count = 0;
        return;
    }

    if(part.vbo == 0) {
        glGenBuffers(1, &part.vbo);
    }

    glBindBuffer(GL_ARRAY_BUFFER, part.vbo);
    glBufferData(GL_ARRAY_BUFFER, std::span(part.quads).size_bytes(), part.quads.data(), GL_STATIC_DRAW);
}

MeshingTask::MeshingTask(entt::entity entity, const ChunkPos& cpos) : m_entity(entity), m_cpos(cpos)
{
    m_cache.init(cpos);
}

void MeshingTask::process(void)
{
    for(std::size_t i = 0; i < constant::CHUNK_VOLUME; ++i) {
        if(status.load(std::memory_order_relaxed) == task_status::CANCELLED) {
            m_opaque.clear();
            m_alpha.clear();
            m_fluid.clear();
            return;
        }

        auto lpos = utils::to_local(i);
        mesh_block(lpos, m_cache.get(lpos));
    }
}

void MeshingTask::finalize(void)
{
    if(!world::chunk_entities.valid(m_entity) || !world::chunk_entities.all_of<Chunk_Component>(m_entity)) {
        s_pending.erase(m_cpos);
        return;
    }

    const auto& chunk = world::chunk_entities.get<Chunk_Component>(m_entity);

    if(chunk.position != m_cpos) {
        s_pending.erase(m_cpos);
        return;
    }

    if(m_opaque.empty() && m_alpha.empty() && m_fluid.empty()) {
        world::chunk_entities.remove<ChunkMesh>(m_entity);
        s_pending.erase(m_cpos);
        return;
    }

    auto& component = world::chunk_entities.get_or_emplace<ChunkMesh>(m_entity);

    component.opaque.quads = std::move(m_opaque);
    component.opaque.count = static_cast<std::uint32_t>(component.opaque.quads.size());
    sync_part(component.opaque);

    component.alpha.quads = std::move(m_alpha);
    component.alpha.count = static_cast<std::uint32_t>(component.alpha.quads.size());
    sync_part(component.alpha);

    component.fluid.quads = std::move(m_fluid);
    component.fluid.count = static_cast<std::uint32_t>(component.fluid.quads.size());
    sync_part(component.fluid);

    // Opaque quads won't get depth-sorted
    // at runtime so there's no point in keeping
    // them persistent in the system memory
    component.opaque.quads.clear();
    component.opaque.quads.shrink_to_fit();

    world::chunk_entities.patch<ChunkMesh>(m_entity);

    s_pending.erase(m_cpos);
}

std::uint32_t MeshingTask::calculate_ao(const LocalPos& lpos, block_face face, const Eigen::Vector3f& vertex) const
{
    auto delta = face_delta(face);
    LocalPos::value_type nx = lpos.x() + delta.x();
    LocalPos::value_type ny = lpos.y() + delta.y();
    LocalPos::value_type nz = lpos.z() + delta.z();

    LocalPos::value_type dx = vertex.x() > 0.5f ? 1 : -1;
    LocalPos::value_type dy = vertex.y() > 0.5f ? 1 : -1;
    LocalPos::value_type dz = vertex.z() > 0.5f ? 1 : -1;

    auto s1x = nx;
    auto s1y = ny;
    auto s1z = nz;

    auto s2x = nx;
    auto s2y = ny;
    auto s2z = nz;

    if(delta.x()) {
        s1y += dy;
        s2z += dz;
    }
    else if(delta.y()) {
        s1x += dx;
        s2z += dz;
    }
    else {
        s1x += dx;
        s2y += dy;
    }

    auto cx = s1x + s2x - nx;
    auto cy = s1y + s2y - ny;
    auto cz = s1z + s2z - nz;

    auto oc_s1 = is_occluder(LocalPos(s1x, s1y, s1z), face);
    auto oc_s2 = is_occluder(LocalPos(s2x, s2y, s2z), face);
    auto oc_c = is_occluder(LocalPos(cx, cy, cz), face);

    if(oc_s1 && oc_s2)
        return 0;
    return 3 - (oc_s1 + oc_s2 + oc_c);
}

bool MeshingTask::is_culled_block(const LocalPos& lpos, block_face face, block_render self_render) const
{
    auto neighbour_lpos = lpos + face_delta(face);
    auto neighbour_id = m_cache.get(neighbour_lpos);

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

bool MeshingTask::is_culled_fluid(const LocalPos& lpos, block_face face, fluid_id_type fluid_id) const
{
    auto neighbour_lpos = lpos + face_delta(face);
    auto neighbour_id = m_cache.get(neighbour_lpos);

    if(neighbour_id == BLOCK_ID_NULL) {
        return false;
    }

    auto neighbour_def = block_registry::find_definition(neighbour_id);

    if(neighbour_def == nullptr) {
        return false;
    }

    if(neighbour_def->fluid == fluid_id) {
        // Top/bottom against the same fluid are always merged. Side faces
        // may still need a gap strip, these are handled in mesh_fluid
        return face == BLOCK_FACE_TOP || face == BLOCK_FACE_BOTTOM;
    }

    if(neighbour_def->render != BLOCK_RENDER_SOLID) {
        return false;
    }

    auto neighbour_baked = block_models::find(neighbour_id);

    if(neighbour_baked == nullptr) {
        return false;
    }

    return neighbour_baked->fully_covered[opposite_face(face)];
}

bool MeshingTask::is_occluder(const LocalPos& lpos, block_face exposed_face) const
{
    auto id = m_cache.get(lpos);

    if(id == BLOCK_ID_NULL) {
        return false;
    }

    auto def = block_registry::find_definition(id);

    if(def == nullptr || def->render != BLOCK_RENDER_SOLID) {
        return false;
    }

    auto baked = block_models::find(id);

    if(baked == nullptr) {
        return false;
    }

    return baked->fully_covered[opposite_face(exposed_face)];
}

static float fluid_surface_height(const BlockCache& cache, const LocalPos& lpos, const BlockDefinition& def, fluid_gravity gravity)
{
    auto height = 0.0625f * static_cast<float>(def.fluid_level);

    if(height <= 0.0f) {
        return 0.0f;
    }

    block_face anti_gravity;

    if(gravity == FLUID_GRAVITY_DOWN) {
        anti_gravity = BLOCK_FACE_TOP;
    }
    else {
        anti_gravity = BLOCK_FACE_BOTTOM;
    }

    auto above_id = cache.get(lpos + face_delta(anti_gravity));
    auto above_def = block_registry::find_definition(above_id);

    if(above_def == nullptr) {
        return height;
    }

    if(above_def->fluid == def.fluid) {
        return 1.0f;
    }

    if(above_def->render == BLOCK_RENDER_SOLID) {
        auto above_baked = block_models::find(above_id);

        if(above_baked && above_baked->fully_covered[opposite_face(anti_gravity)]) {
            return 1.0f;
        }
    }

    return height;
}

void MeshingTask::emit_block_quads(std::vector<ChunkMesh_Quad>& out, std::span<const BakedBlockModel_Quad> quads, const LocalPos& lpos,
    std::uint64_t entropy, std::optional<block_face> face) const
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

        std::array<std::uint32_t, 4> ao_values;
        ao_values.fill(3);

        if(face.has_value()) {
            for(int i = 0; i < 4; ++i) {
                ao_values[i] = calculate_ao(lpos, face.value(), quad.positions[i]);
            }
        }

        out_quad.data_extras = ChunkMesh_Quad::pack_extras(ao_values);

        out.push_back(out_quad);
    }
}

void MeshingTask::emit_fluid_quad(std::vector<ChunkMesh_Quad>& out, const LocalPos& lpos, const Eigen::Vector3f& min,
    const Eigen::Vector3f& max, block_face face, const AtlasStrip* strip, unsigned tint_index) const
{
    if(strip == nullptr) {
        return;
    }

    std::array<Eigen::Vector3f, 4> positions {};
    std::array<Eigen::Vector2f, 4> uvs {};

    switch(face) {
        case BLOCK_FACE_NORTH:
            positions[0] = Eigen::Vector3f(min.x(), max.y(), min.z());
            positions[1] = Eigen::Vector3f(max.x(), max.y(), min.z());
            positions[2] = Eigen::Vector3f(max.x(), min.y(), min.z());
            positions[3] = Eigen::Vector3f(min.x(), min.y(), min.z());
            break;

        case BLOCK_FACE_SOUTH:
            positions[0] = Eigen::Vector3f(max.x(), max.y(), max.z());
            positions[1] = Eigen::Vector3f(min.x(), max.y(), max.z());
            positions[2] = Eigen::Vector3f(min.x(), min.y(), max.z());
            positions[3] = Eigen::Vector3f(max.x(), min.y(), max.z());
            break;

        case BLOCK_FACE_EAST:
            positions[0] = Eigen::Vector3f(max.x(), max.y(), min.z());
            positions[1] = Eigen::Vector3f(max.x(), max.y(), max.z());
            positions[2] = Eigen::Vector3f(max.x(), min.y(), max.z());
            positions[3] = Eigen::Vector3f(max.x(), min.y(), min.z());
            break;

        case BLOCK_FACE_WEST:
            positions[0] = Eigen::Vector3f(min.x(), max.y(), max.z());
            positions[1] = Eigen::Vector3f(min.x(), max.y(), min.z());
            positions[2] = Eigen::Vector3f(min.x(), min.y(), min.z());
            positions[3] = Eigen::Vector3f(min.x(), min.y(), max.z());
            break;

        case BLOCK_FACE_TOP:
            positions[0] = Eigen::Vector3f(min.x(), max.y(), min.z());
            positions[1] = Eigen::Vector3f(min.x(), max.y(), max.z());
            positions[2] = Eigen::Vector3f(max.x(), max.y(), max.z());
            positions[3] = Eigen::Vector3f(max.x(), max.y(), min.z());
            break;

        case BLOCK_FACE_BOTTOM:
            positions[0] = Eigen::Vector3f(min.x(), min.y(), max.z());
            positions[1] = Eigen::Vector3f(min.x(), min.y(), min.z());
            positions[2] = Eigen::Vector3f(max.x(), min.y(), min.z());
            positions[3] = Eigen::Vector3f(max.x(), min.y(), max.z());
            break;
    }

    switch(face) {
        case BLOCK_FACE_TOP:
            uvs[0] = Eigen::Vector2f(0.0f, 0.0f);
            uvs[1] = Eigen::Vector2f(0.0f, 1.0f);
            uvs[2] = Eigen::Vector2f(1.0f, 1.0f);
            uvs[3] = Eigen::Vector2f(1.0f, 0.0f);
            break;

        case BLOCK_FACE_BOTTOM:
            uvs[0] = Eigen::Vector2f(1.0f, 1.0f);
            uvs[1] = Eigen::Vector2f(1.0f, 0.0f);
            uvs[2] = Eigen::Vector2f(0.0f, 0.0f);
            uvs[3] = Eigen::Vector2f(0.0f, 1.0f);
            break;

        default:
            uvs[0] = Eigen::Vector2f(1.0f, 0.0f);
            uvs[1] = Eigen::Vector2f(0.0f, 0.0f);
            uvs[2] = Eigen::Vector2f(0.0f, 1.0f);
            uvs[3] = Eigen::Vector2f(1.0f, 1.0f);
            break;
    }

    auto block_origin = 16.0f * lpos.cast<float>();
    Eigen::Vector3f p0 = block_origin + 16.0f * positions[0];
    Eigen::Vector3f p1 = block_origin + 16.0f * positions[1];
    Eigen::Vector3f p3 = block_origin + 16.0f * positions[3];

    const auto& c0 = uvs[0];
    const auto& c2 = uvs[2];

    auto dist_c0 = std::abs(uvs[1].x() - c0.x());
    auto dist_c2 = std::abs(uvs[1].x() - c2.x());
    auto uv_orient_flag = dist_c0 < dist_c2;

    ChunkMesh_Quad out_quad {};
    out_quad.data_origin = ChunkMesh_Quad::pack_position(p0);
    out_quad.data_edge_u = ChunkMesh_Quad::pack_offset(p1 - p0) | ChunkMesh_Quad::ANIMATED_BIT;
    out_quad.data_edge_v = ChunkMesh_Quad::pack_offset(p3 - p0);

    if(uv_orient_flag) {
        out_quad.data_origin |= ChunkMesh_Quad::UV_ORIENT_BIT;
    }

    out_quad.data_uv = ChunkMesh_Quad::pack_uv(c0, c2);
    out_quad.data_texture = ChunkMesh_Quad::pack_texture(static_cast<std::uint32_t>(strip->index), 0, tint_index);

    std::array<std::uint32_t, 4> ao_values;
    ao_values.fill(3);

    out_quad.data_extras = ChunkMesh_Quad::pack_extras(ao_values);

    out.push_back(out_quad);
}

void MeshingTask::mesh_fluid(const LocalPos& lpos, const BlockDefinition& def)
{
    auto fluid = fluid_registry::find_definition(def.fluid);
    auto cached = fluid_cache::find(def.fluid);

    if(fluid == nullptr || cached == nullptr) {
        return;
    }

    auto height = 0.0625f * static_cast<float>(def.fluid_level);

    if(height <= 0.0f) {
        return;
    }

    auto side_height = fluid_surface_height(m_cache, lpos, def, fluid->gravity);
    auto& bucket = fluid->opaque ? m_opaque : m_fluid;

    for(auto face : ALL_FACES) {
        auto is_side = false;
        is_side = is_side || face == BLOCK_FACE_NORTH;
        is_side = is_side || face == BLOCK_FACE_SOUTH;
        is_side = is_side || face == BLOCK_FACE_EAST;
        is_side = is_side || face == BLOCK_FACE_WEST;

        if(is_side) {
            auto neighbour_lpos = lpos + face_delta(face);
            auto neighbour_id = m_cache.get(neighbour_lpos);
            auto neighbour_def = block_registry::find_definition(neighbour_id);

            if(neighbour_def && neighbour_def->fluid == def.fluid) {
                auto neighbour_height = fluid_surface_height(m_cache, neighbour_lpos, *neighbour_def, fluid->gravity);

                if(side_height <= neighbour_height) {
                    continue;
                }

                Eigen::Vector3f min = Eigen::Vector3f::Zero();
                Eigen::Vector3f max = Eigen::Vector3f::Ones();

                if(fluid->gravity == FLUID_GRAVITY_DOWN) {
                    min.y() = neighbour_height;
                    max.y() = side_height;
                }
                else {
                    min.y() = 1.0f - side_height;
                    max.y() = 1.0f - neighbour_height;
                }

                emit_fluid_quad(bucket, lpos, min, max, face, cached->flowing, fluid->tint_index.value_or(0));

                continue;
            }
        }

        if(is_culled_fluid(lpos, face, def.fluid)) {
            continue;
        }

        float face_height;

        if(is_side) {
            face_height = side_height;
        }
        else {
            face_height = height;
        }

        Eigen::Vector3f min = Eigen::Vector3f::Zero();
        Eigen::Vector3f max = Eigen::Vector3f::Ones();

        if(fluid->gravity == FLUID_GRAVITY_DOWN) {
            max.y() = face_height;
        }
        else {
            min.y() = 1.0f - face_height;
        }

        if(is_side) {
            emit_fluid_quad(bucket, lpos, min, max, face, cached->flowing, fluid->tint_index.value_or(0));
        }
        else {
            emit_fluid_quad(bucket, lpos, min, max, face, cached->still, fluid->tint_index.value_or(0));
        }
    }
}

void MeshingTask::mesh_block(const LocalPos& lpos, block_id_type id)
{
    if(id == BLOCK_ID_NULL) {
        return;
    }

    auto def = block_registry::find_definition(id);

    if(def == nullptr) {
        return;
    }

    auto baked = block_models::find(id);

    if(baked && def->render) {
        std::array<bool, 6> culled {};
        auto fully_enclosed = true;

        for(auto face : ALL_FACES) {
            culled[face] = is_culled_block(lpos, face, def->render);
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
                emit_block_quads(m_alpha, baked->unculled_quads, lpos, entropy, std::nullopt);
            }

            for(auto face : ALL_FACES) {
                if(culled[face])
                    continue;
                emit_block_quads(m_alpha, baked->face_quads[face], lpos, entropy, face);
            }
        }
        else {
            if(!fully_enclosed) {
                emit_block_quads(m_opaque, baked->unculled_quads, lpos, entropy, std::nullopt);
            }

            for(auto face : ALL_FACES) {
                if(culled[face])
                    continue;
                emit_block_quads(m_opaque, baked->face_quads[face], lpos, entropy, face);
            }
        }
    }

    if(def->fluid) {
        mesh_fluid(lpos, *def);
    }
}

static bool is_neighbour(const LocalPos& lpos)
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

static void mark_dirty(const ChunkPos& cpos)
{
    if(auto chunk = world::find_chunk(cpos)) {
        mark_dirty(chunk->entity());
    }
}

static bool should_remesh_neighbor(const ChunkPos& cpos)
{
    if(s_pending.contains(cpos)) {
        return true;
    }

    auto chunk = world::find_chunk(cpos);

    if(chunk == nullptr) {
        return false;
    }

    return world::chunk_entities.all_of<ChunkMesh>(chunk->entity());
}

static void mark_neighbors_dirty(const ChunkPos& cpos)
{
    for(auto face : ALL_FACES) {
        auto npos = cpos + face_delta(face);

        if(should_remesh_neighbor(npos)) {
            mark_dirty(npos);
        }
    }
}

static void mark_border_neighbors_dirty(const ChunkPos& cpos, const LocalPos& lpos)
{
    for(auto face : ALL_FACES) {
        if(is_neighbour((lpos + face_delta(face)).eval())) {
            auto npos = cpos + face_delta(face);

            if(should_remesh_neighbor(npos)) {
                mark_dirty(npos);
            }
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
        if(0 == s_pending.count(chunk.position)) {
            s_pending.emplace(chunk.position, nullptr);
            world::chunk_entities.remove<ChunkMesh_DirtyMarker>(entity);
            threading::submit<MeshingTask>(entity, chunk.position);

            count += 1;

            if(count >= THROTTLE_COUNT) {
                break;
            }
        }
    }
}
