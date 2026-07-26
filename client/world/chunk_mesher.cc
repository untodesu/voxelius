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
    bool is_culled_fluid(const LocalPos& lpos, block_face face, fluid_id_type fluid_id, bool face_flush) const;
    bool is_occluder(const LocalPos& lpos, block_face exposed_face) const;

    void emit_block_quads(std::vector<ChunkMesh_Vertex>& out, std::span<const BakedBlockModel_Quad> quads, const LocalPos& lpos,
        std::uint64_t entropy, std::optional<block_face> face) const;
    void emit_fluid_face(std::vector<ChunkMesh_Vertex>& out, const LocalPos& lpos, const std::array<Eigen::Vector3f, 4>& positions,
        block_face face, const AtlasStrip* strip, unsigned tint_index,
        std::optional<std::array<Eigen::Vector2f, 4>> uvs_override = std::nullopt) const;

    void mesh_fluid(const LocalPos& lpos, const BlockDefinition& def);
    void mesh_block(const LocalPos& lpos, block_id_type id);

    BlockCache m_cache;
    std::vector<ChunkMesh_Vertex> m_opaque;
    std::vector<ChunkMesh_Vertex> m_alpha;
    std::vector<ChunkMesh_Vertex> m_fluid;
    entt::entity m_entity;
    ChunkPos m_cpos;
};

static float shade_factor(const Eigen::Vector3f& normal)
{
    auto ax = std::abs(normal.x());
    auto ay = std::abs(normal.y());
    auto az = std::abs(normal.z());

    if(ay >= ax && ay >= az) {
        if(normal.y()) {
            return 1.0f;
        }
        else {
            return 0.4f;
        }
    }

    if(ax >= az) {
        return 0.6f;
    }
    else {
        return 0.8f;
    }
}

static void emit_quad_vertices(std::vector<ChunkMesh_Vertex>& out, const std::array<Eigen::Vector3f, 4>& positions_16ths,
    const std::array<Eigen::Vector2f, 4>& uvs, const std::array<std::uint32_t, 4>& ao_values, std::uint32_t texture, float shade,
    bool animated)
{
    auto flip_quad = false;
    std::array<int, 4> order;

    if(ao_values[0] + ao_values[2] < ao_values[1] + ao_values[3]) {
        order[0] = 0;
        order[1] = 1;
        order[2] = 3;
        order[3] = 2;
    }
    else {
        order[0] = 1;
        order[1] = 2;
        order[2] = 0;
        order[3] = 3;
    }

    for(auto corner : order) {
        ChunkMesh_Vertex vertex {};
        vertex.data_position = ChunkMesh_Vertex::pack_position(positions_16ths[corner]);
        vertex.data_uv = ChunkMesh_Vertex::pack_uv(uvs[corner]);
        vertex.data_texture = texture;
        vertex.data_extras = ChunkMesh_Vertex::pack_extras(ao_values[corner], shade, animated);
        out.push_back(vertex);
    }
}

static void sync_part(ChunkMesh_Part& part)
{
    if(part.vertices.empty()) {
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
    glBufferData(GL_ARRAY_BUFFER, std::span(part.vertices).size_bytes(), part.vertices.data(), GL_STATIC_DRAW);
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

    component.opaque.vertices = std::move(m_opaque);
    component.opaque.count = static_cast<std::uint32_t>(component.opaque.vertices.size());
    sync_part(component.opaque);

    component.alpha.vertices = std::move(m_alpha);
    component.alpha.count = static_cast<std::uint32_t>(component.alpha.vertices.size());
    sync_part(component.alpha);

    component.fluid.vertices = std::move(m_fluid);
    component.fluid.count = static_cast<std::uint32_t>(component.fluid.vertices.size());
    sync_part(component.fluid);

    // Opaque vertices won't get depth-sorted
    // at runtime so there's no point in keeping
    // them persistent in the system memory
    component.opaque.vertices.clear();
    component.opaque.vertices.shrink_to_fit();

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

bool MeshingTask::is_culled_fluid(const LocalPos& lpos, block_face face, fluid_id_type fluid_id, bool face_flush) const
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

    if(neighbour_def->fluid == fluid_id && neighbour_def->fluid_level > 0) {
        return face == BLOCK_FACE_TOP || face == BLOCK_FACE_BOTTOM;
    }

    if(face_flush) {
        if(neighbour_def->render != BLOCK_RENDER_SOLID) {
            return false;
        }

        auto neighbour_baked = block_models::find(neighbour_id);

        if(neighbour_baked == nullptr) {
            return false;
        }

        return neighbour_baked->fully_covered[opposite_face(face)];
    }

    return false;
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

void MeshingTask::emit_block_quads(std::vector<ChunkMesh_Vertex>& out, std::span<const BakedBlockModel_Quad> quads, const LocalPos& lpos,
    std::uint64_t entropy, std::optional<block_face> face) const
{
    auto block_origin = lpos.cast<float>() * 16.0f;

    for(const auto& quad : quads) {
        std::uint32_t frame_offset = 0;

        if(quad.frame_count > 0 && !quad.animated) {
            frame_offset = static_cast<std::uint32_t>(entropy % quad.frame_count);
        }

        std::array<Eigen::Vector3f, 4> positions_16ths;
        positions_16ths[0] = block_origin + 16.0f * quad.positions[0];
        positions_16ths[1] = block_origin + 16.0f * quad.positions[1];
        positions_16ths[2] = block_origin + 16.0f * quad.positions[2];
        positions_16ths[3] = block_origin + 16.0f * quad.positions[3];

        std::array<std::uint32_t, 4> ao_values;
        ao_values.fill(3);

        if(face.has_value()) {
            for(int i = 0; i < 4; ++i) {
                ao_values[i] = calculate_ao(lpos, face.value(), quad.positions[i]);
            }
        }

        auto shade = 1.0f;

        if(quad.shade) {
            auto edge_u = positions_16ths[1] - positions_16ths[0];
            auto edge_v = positions_16ths[3] - positions_16ths[0];
            shade = shade_factor(edge_u.cross(edge_v).normalized());
        }

        auto texture = ChunkMesh_Vertex::pack_texture(quad.texture_index, frame_offset, quad.tint_index);

        emit_quad_vertices(out, positions_16ths, quad.uvs, ao_values, texture, shade, quad.animated);
    }
}

static float fluid_surface_height(const BlockCache& cache, const LocalPos& lpos, const BlockDefinition* def, fluid_gravity gravity)
{
    auto height = 0.0625f * static_cast<float>(def->fluid_level);

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

    if(above_def && above_def->fluid == def->fluid && above_def->fluid_level > 0) {
        return 1.0f;
    }

    constexpr unsigned FULL_FLUID_LEVEL = 14U; // TODO: make this a value in FluidDefinition

    if(def->fluid_level >= FULL_FLUID_LEVEL && above_def && above_def->render == BLOCK_RENDER_SOLID) {
        auto above_baked = block_models::find(above_id);
        auto face = opposite_face(anti_gravity);

        if(above_baked && above_baked->fully_covered[face]) {
            return 1.0f;
        }
    }

    return height;
}

static std::optional<float> fluid_cell_height(const BlockCache& cache, const LocalPos& lpos, fluid_id_type fluid_id, fluid_gravity gravity)
{
    auto def = block_registry::find_definition(cache.get(lpos));

    if(def == nullptr || def->fluid != fluid_id || def->fluid_level == 0) {
        return std::nullopt;
    }

    return fluid_surface_height(cache, lpos, def, gravity);
}

static float fluid_corner_height(const BlockCache& cache, const LocalPos& lpos, fluid_id_type fluid_id, fluid_gravity gravity, int sx,
    int sz, float self_height)
{
    auto sum = 0.0f;
    auto count = 0;

    for(int dx = sx - 1; dx <= sx; ++dx) {
        for(int dz = sz - 1; dz <= sz; ++dz) {
            auto height = fluid_cell_height(cache, lpos + LocalPos(dx, 0, dz), fluid_id, gravity);

            if(!height.has_value()) {
                continue;
            }

            if(height.value() >= 1.0f) {
                return 1.0f;
            }

            sum += height.value();
            count += 1;
        }
    }

    if(count == 0) {
        return self_height;
    }

    return sum / static_cast<float>(count);
}

static std::array<float, 4> fluid_corner_heights(const BlockCache& cache, const LocalPos& lpos, fluid_id_type fluid_id,
    fluid_gravity gravity, float self_height)
{
    return {
        fluid_corner_height(cache, lpos, fluid_id, gravity, 0, 0, self_height),
        fluid_corner_height(cache, lpos, fluid_id, gravity, 0, 1, self_height),
        fluid_corner_height(cache, lpos, fluid_id, gravity, 1, 1, self_height),
        fluid_corner_height(cache, lpos, fluid_id, gravity, 1, 0, self_height),
    };
}

static float fluid_surface_y(float corner_height, fluid_gravity gravity)
{
    if(gravity == FLUID_GRAVITY_DOWN) {
        return corner_height;
    }

    return 1.0f - corner_height;
}

static float fluid_floor_y(fluid_gravity gravity)
{
    if(gravity == FLUID_GRAVITY_DOWN) {
        return 0.0f;
    }

    return 1.0f;
}

static std::optional<float> fluid_neighbour_level(const BlockCache& cache, const LocalPos& lpos, fluid_id_type fluid_id)
{
    auto def = block_registry::find_definition(cache.get(lpos));

    if(def == nullptr || def->fluid != fluid_id || def->fluid_level == 0) {
        return std::nullopt;
    }

    return 0.0625f * static_cast<float>(def->fluid_level);
}

static std::array<Eigen::Vector2f, 4> flowing_top_uvs(float flow_x, float flow_z)
{
    std::array<Eigen::Vector2f, 4> uvs = {
        Eigen::Vector2f(0.0f, 0.0f),
        Eigen::Vector2f(0.0f, 1.0f),
        Eigen::Vector2f(1.0f, 1.0f),
        Eigen::Vector2f(1.0f, 0.0f),
    };

    auto len_sq = flow_x * flow_x + flow_z * flow_z;

    if(len_sq <= 1.0e-8f) {
        return uvs;
    }

    auto angle = std::atan2(flow_z, flow_x);
    auto quarter = static_cast<int>(std::lround(angle / (0.5f * 3.14159265f)));
    quarter = ((quarter % 4) + 4) % 4;

    switch(quarter) {
        case 0:
            uvs[0] = Eigen::Vector2f(0.0f, 0.0f);
            uvs[1] = Eigen::Vector2f(1.0f, 0.0f);
            uvs[2] = Eigen::Vector2f(1.0f, 1.0f);
            uvs[3] = Eigen::Vector2f(0.0f, 1.0f);
            break;

        case 2:
            uvs[0] = Eigen::Vector2f(1.0f, 0.0f);
            uvs[1] = Eigen::Vector2f(0.0f, 1.0f);
            uvs[2] = Eigen::Vector2f(0.0f, 0.0f);
            uvs[3] = Eigen::Vector2f(1.0f, 0.0f);
            break;

        case 3:
            uvs[0] = Eigen::Vector2f(1.0f, 1.0f);
            uvs[1] = Eigen::Vector2f(1.0f, 0.0f);
            uvs[2] = Eigen::Vector2f(0.0f, 0.0f);
            uvs[3] = Eigen::Vector2f(0.0f, 1.0f);
            break;
    }

    return uvs;
}

void MeshingTask::emit_fluid_face(std::vector<ChunkMesh_Vertex>& out, const LocalPos& lpos, const std::array<Eigen::Vector3f, 4>& positions,
    block_face face, const AtlasStrip* strip, unsigned tint_index, std::optional<std::array<Eigen::Vector2f, 4>> uvs_override) const
{
    if(strip == nullptr) {
        return;
    }

    std::array<Eigen::Vector2f, 4> uvs {};

    if(uvs_override.has_value()) {
        uvs = uvs_override.value();
    }
    else {
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
    }

    auto block_origin = 16.0f * lpos.cast<float>();

    std::array<Eigen::Vector3f, 4> positions_16ths;
    positions_16ths[0] = block_origin + 16.0f * positions[0];
    positions_16ths[1] = block_origin + 16.0f * positions[1];
    positions_16ths[2] = block_origin + 16.0f * positions[2];
    positions_16ths[3] = block_origin + 16.0f * positions[3];

    std::array<std::uint32_t, 4> ao_values;
    ao_values.fill(3);

    auto strip_index = static_cast<std::uint32_t>(strip->index);
    auto texture = ChunkMesh_Vertex::pack_texture(strip_index, 0, tint_index);

    emit_quad_vertices(out, positions_16ths, uvs, ao_values, texture, 1.0f, true);
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

    auto surface_height = fluid_surface_height(m_cache, lpos, &def, fluid->gravity);
    auto corners = fluid_corner_heights(m_cache, lpos, def.fluid, fluid->gravity, surface_height);
    auto& bucket = fluid->opaque ? m_opaque : m_fluid;
    auto tint_index = fluid->tint_index.value_or(0);

    constexpr unsigned FULL_FLUID_LEVEL = 14U; // TODO: make this a value in FluidDefinition

    auto use_still = def.fluid_level >= FULL_FLUID_LEVEL;
    auto flow_x = 0.0f;
    auto flow_z = 0.0f;

    if(!use_still) {
        const std::array dirs = {
            LocalPos(-1, 0, 0),
            LocalPos(+1, 0, 0),
            LocalPos(0, 0, -1),
            LocalPos(0, 0, +1),
        };

        for(const auto& dir : dirs) {
            auto neighbour_h = fluid_neighbour_level(m_cache, lpos + dir, def.fluid);

            if(!neighbour_h.has_value()) {
                continue;
            }

            auto delta = height - neighbour_h.value();
            flow_x += delta * static_cast<float>(dir.x());
            flow_z += delta * static_cast<float>(dir.z());
        }
    }

    auto floor_y = fluid_floor_y(fluid->gravity);
    auto y0 = fluid_surface_y(corners[0], fluid->gravity);
    auto y1 = fluid_surface_y(corners[1], fluid->gravity);
    auto y2 = fluid_surface_y(corners[2], fluid->gravity);
    auto y3 = fluid_surface_y(corners[3], fluid->gravity);

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

            if(neighbour_def && neighbour_def->fluid == def.fluid && neighbour_def->fluid_level > 0) {
                continue;
            }
        }

        auto surface_face = !is_side;
        auto face_flush = true;

        if(fluid->gravity == FLUID_GRAVITY_DOWN) {
            surface_face = surface_face && face == BLOCK_FACE_TOP;
        }
        else {
            surface_face = surface_face && face == BLOCK_FACE_BOTTOM;
        }

        if(surface_face) {
            face_flush = true;

            for(float corner : corners) {
                if(corner < 1.0f) {
                    face_flush = false;
                    break;
                }
            }
        }

        if(is_culled_fluid(lpos, face, def.fluid, face_flush)) {
            continue;
        }

        std::array<Eigen::Vector3f, 4> positions {};

        switch(face) {
            case BLOCK_FACE_NORTH:
                positions[0] = Eigen::Vector3f(0.0f, y0, 0.0f);
                positions[1] = Eigen::Vector3f(1.0f, y3, 0.0f);
                positions[2] = Eigen::Vector3f(1.0f, floor_y, 0.0f);
                positions[3] = Eigen::Vector3f(0.0f, floor_y, 0.0f);
                break;

            case BLOCK_FACE_SOUTH:
                positions[0] = Eigen::Vector3f(1.0f, y2, 1.0f);
                positions[1] = Eigen::Vector3f(0.0f, y1, 1.0f);
                positions[2] = Eigen::Vector3f(0.0f, floor_y, 1.0f);
                positions[3] = Eigen::Vector3f(1.0f, floor_y, 1.0f);
                break;

            case BLOCK_FACE_EAST:
                positions[0] = Eigen::Vector3f(1.0f, y3, 0.0f);
                positions[1] = Eigen::Vector3f(1.0f, y2, 1.0f);
                positions[2] = Eigen::Vector3f(1.0f, floor_y, 1.0f);
                positions[3] = Eigen::Vector3f(1.0f, floor_y, 0.0f);
                break;

            case BLOCK_FACE_WEST:
                positions[0] = Eigen::Vector3f(0.0f, y1, 1.0f);
                positions[1] = Eigen::Vector3f(0.0f, y0, 0.0f);
                positions[2] = Eigen::Vector3f(0.0f, floor_y, 0.0f);
                positions[3] = Eigen::Vector3f(0.0f, floor_y, 1.0f);
                break;

            case BLOCK_FACE_TOP:
                if(fluid->gravity == FLUID_GRAVITY_DOWN) {
                    positions[0] = Eigen::Vector3f(0.0f, y0, 0.0f);
                    positions[1] = Eigen::Vector3f(0.0f, y1, 1.0f);
                    positions[2] = Eigen::Vector3f(1.0f, y2, 1.0f);
                    positions[3] = Eigen::Vector3f(1.0f, y3, 0.0f);
                }
                else {
                    positions[0] = Eigen::Vector3f(0.0f, 1.0f, 0.0f);
                    positions[1] = Eigen::Vector3f(0.0f, 1.0f, 1.0f);
                    positions[2] = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
                    positions[3] = Eigen::Vector3f(1.0f, 1.0f, 0.0f);
                }
                break;

            case BLOCK_FACE_BOTTOM:
                if(fluid->gravity == FLUID_GRAVITY_UP) {
                    positions[0] = Eigen::Vector3f(0.0f, y0, 1.0f);
                    positions[1] = Eigen::Vector3f(0.0f, y1, 0.0f);
                    positions[2] = Eigen::Vector3f(1.0f, y2, 0.0f);
                    positions[3] = Eigen::Vector3f(1.0f, y3, 1.0f);
                }
                else {
                    positions[0] = Eigen::Vector3f(0.0f, 0.0f, 1.0f);
                    positions[1] = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
                    positions[2] = Eigen::Vector3f(1.0f, 0.0f, 0.0f);
                    positions[3] = Eigen::Vector3f(1.0f, 0.0f, 1.0f);
                }
                break;
        }

        if(is_side) {
            emit_fluid_face(bucket, lpos, positions, face, cached->flowing, tint_index);
        }
        else if(surface_face) {
            const AtlasStrip* strip;

            if(use_still) {
                strip = cached->still;
            }
            else if(cached->flowing) {
                strip = cached->flowing;
            }
            else {
                strip = cached->still;
            }

            std::optional<std::array<Eigen::Vector2f, 4>> uvs;

            if(!use_still && cached->flowing) {
                uvs = flowing_top_uvs(flow_x, flow_z);
            }

            emit_fluid_face(bucket, lpos, positions, face, strip, tint_index, uvs);
        }
        else {
            emit_fluid_face(bucket, lpos, positions, face, cached->still, tint_index);
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

static bool touches_neighbor_chunk(const LocalPos& lpos, ChunkPos::value_type dx, ChunkPos::value_type dy, ChunkPos::value_type dz)
{
    constexpr auto size = static_cast<LocalPos::value_type>(constant::CHUNK_SIZE);

    if(dx < 0 && lpos.x() > 0) {
        return false;
    }

    if(dx > 0 && lpos.x() < size - 1) {
        return false;
    }

    if(dy < 0 && lpos.y() > 0) {
        return false;
    }

    if(dy > 0 && lpos.y() < size - 1) {
        return false;
    }

    if(dz < 0 && lpos.z() > 0) {
        return false;
    }

    if(dz > 0 && lpos.z() < size - 1) {
        return false;
    }

    return true;
}

static void mark_neighbors_dirty(const ChunkPos& cpos)
{
    for(ChunkPos::value_type dz = -1; dz <= 1; ++dz) {
        for(ChunkPos::value_type dy = -1; dy <= 1; ++dy) {
            for(ChunkPos::value_type dx = -1; dx <= 1; ++dx) {
                if(dx == 0 && dy == 0 && dz == 0) {
                    continue;
                }

                mark_dirty(cpos + ChunkPos(dx, dy, dz));
            }
        }
    }
}

static void mark_border_neighbors_dirty(const ChunkPos& cpos, const LocalPos& lpos)
{
    for(ChunkPos::value_type dz = -1; dz <= 1; ++dz) {
        for(ChunkPos::value_type dy = -1; dy <= 1; ++dy) {
            for(ChunkPos::value_type dx = -1; dx <= 1; ++dx) {
                if(dx == 0 && dy == 0 && dz == 0) {
                    continue;
                }

                if(!touches_neighbor_chunk(lpos, dx, dy, dz)) {
                    continue;
                }

                mark_dirty(cpos + ChunkPos(dx, dy, dz));
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
