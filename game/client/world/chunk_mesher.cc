#include "client/pch.hh"

#include "client/world/chunk_mesher.hh"

#include "core/math/crc64.hh"

#include "core/threading.hh"

#include "shared/world/chunk.hh"
#include "shared/world/dimension.hh"
#include "shared/world/voxel_registry.hh"

#include "shared/coord.hh"

#include "client/world/chunk_quad.hh"
#include "client/world/voxel_atlas.hh"

#include "client/globals.hh"
#include "client/session.hh"

using QuadBuilder = std::vector<world::ChunkQuad>;

using CachedChunkCoord = unsigned short;
constexpr static CachedChunkCoord CPOS_ITSELF = 0x0000;
constexpr static CachedChunkCoord CPOS_NORTH = 0x0001;
constexpr static CachedChunkCoord CPOS_SOUTH = 0x0002;
constexpr static CachedChunkCoord CPOS_EAST = 0x0003;
constexpr static CachedChunkCoord CPOS_WEST = 0x0004;
constexpr static CachedChunkCoord CPOS_TOP = 0x0005;
constexpr static CachedChunkCoord CPOS_BOTTOM = 0x0006;
constexpr static const size_t NUM_CACHED_CPOS = 7;

static const CachedChunkCoord get_cached_cpos(const chunk_pos& pivot, const chunk_pos& cpos)
{
    static const CachedChunkCoord nx[3] = { CPOS_WEST, 0, CPOS_EAST };
    static const CachedChunkCoord ny[3] = { CPOS_BOTTOM, 0, CPOS_TOP };
    static const CachedChunkCoord nz[3] = { CPOS_NORTH, 0, CPOS_SOUTH };

    if(pivot != cpos) {
        chunk_pos delta = pivot - cpos;
        delta[0] = math::clamp<std::int64_t>(delta[0], -1, 1);
        delta[1] = math::clamp<std::int64_t>(delta[1], -1, 1);
        delta[2] = math::clamp<std::int64_t>(delta[2], -1, 1);

        if(delta[0]) {
            return nx[delta[0] + 1];
        }
        else if(delta[1]) {
            return ny[delta[1] + 1];
        }
        else {
            return nz[delta[2] + 1];
        }
    }

    return CPOS_ITSELF;
}

static world::voxel_facing get_facing(world::voxel_face face, world::voxel_type type)
{
    if(type == world::voxel_type::CROSS) {
        switch(face) {
            case world::voxel_face::CROSS_NESW:
                return world::voxel_facing::NESW;
            case world::voxel_face::CROSS_NWSE:
                return world::voxel_facing::NWSE;
            default:
                return world::voxel_facing::NORTH;
        }
    }

    switch(face) {
        case world::voxel_face::CUBE_NORTH:
            return world::voxel_facing::NORTH;
        case world::voxel_face::CUBE_SOUTH:
            return world::voxel_facing::SOUTH;
        case world::voxel_face::CUBE_EAST:
            return world::voxel_facing::EAST;
        case world::voxel_face::CUBE_WEST:
            return world::voxel_facing::WEST;
        case world::voxel_face::CUBE_TOP:
            return world::voxel_facing::UP;
        case world::voxel_face::CUBE_BOTTOM:
            return world::voxel_facing::DOWN;
        default:
            return world::voxel_facing::NORTH;
    }
}

class GL_MeshingTask final : public Task {
public:
    explicit GL_MeshingTask(entt::entity entity, const chunk_pos& cpos);
    virtual ~GL_MeshingTask(void) = default;
    virtual void process(void) override;
    virtual void finalize(void) override;

private:
    bool vis_test(voxel_id voxel, const world::VoxelInfo* info, const local_pos& lpos) const;
    void push_quad_a(const world::VoxelInfo* info, const glm::fvec3& pos, const glm::fvec2& size, world::voxel_face face);
    void push_quad_v(const world::VoxelInfo* info, const glm::fvec3& pos, const glm::fvec2& size, world::voxel_face face,
            std::size_t entropy);
    void make_cube(voxel_id voxel, const world::VoxelInfo* info, const local_pos& lpos, world::voxel_vis vis, std::size_t entropy);
    void cache_chunk(const chunk_pos& cpos);

private:
    std::array<world::VoxelStorage, NUM_CACHED_CPOS> m_cache;
    std::vector<QuadBuilder> m_quads_b; // blending
    std::vector<QuadBuilder> m_quads_s; // solid
    entt::entity m_entity;
    chunk_pos m_cpos;
};

GL_MeshingTask::GL_MeshingTask(entt::entity entity, const chunk_pos& cpos)
{
    m_entity = entity;
    m_cpos = cpos;

    cache_chunk(m_cpos);
    cache_chunk(m_cpos + DIR_NORTH<chunk_pos::value_type>);
    cache_chunk(m_cpos + DIR_SOUTH<chunk_pos::value_type>);
    cache_chunk(m_cpos + DIR_EAST<chunk_pos::value_type>);
    cache_chunk(m_cpos + DIR_WEST<chunk_pos::value_type>);
    cache_chunk(m_cpos + DIR_DOWN<chunk_pos::value_type>);
    cache_chunk(m_cpos + DIR_UP<chunk_pos::value_type>);
}

void GL_MeshingTask::process(void)
{
    m_quads_b.resize(world::voxel_atlas::plane_count());
    m_quads_s.resize(world::voxel_atlas::plane_count());

    const auto& voxels = m_cache.at(CPOS_ITSELF);

    for(std::size_t i = 0; i < CHUNK_VOLUME; ++i) {
        if(m_status == task_status::CANCELLED) {
            m_quads_b.clear();
            m_quads_s.clear();
            return;
        }

        const auto voxel = voxels[i];
        const auto lpos = coord::to_local(i);

        const auto info = world::voxel_registry::find(voxel);

        if(info == nullptr) {
            // Either a NULL_VOXEL_ID or something went
            // horribly wrong and we don't what this is
            continue;
        }

        world::voxel_vis vis = 0;

        if(vis_test(voxel, info, lpos + DIR_NORTH<local_pos::value_type>)) {
            vis |= world::VIS_NORTH;
        }

        if(vis_test(voxel, info, lpos + DIR_SOUTH<local_pos::value_type>)) {
            vis |= world::VIS_SOUTH;
        }

        if(vis_test(voxel, info, lpos + DIR_EAST<local_pos::value_type>)) {
            vis |= world::VIS_EAST;
        }

        if(vis_test(voxel, info, lpos + DIR_WEST<local_pos::value_type>)) {
            vis |= world::VIS_WEST;
        }

        if(vis_test(voxel, info, lpos + DIR_UP<local_pos::value_type>)) {
            vis |= world::VIS_UP;
        }

        if(vis_test(voxel, info, lpos + DIR_DOWN<local_pos::value_type>)) {
            vis |= world::VIS_DOWN;
        }

        const auto vpos = coord::to_voxel(m_cpos, lpos);
        const auto entropy_src = vpos[0] * vpos[1] * vpos[2];
        const auto entropy = math::crc64(&entropy_src, sizeof(entropy_src));

        // FIXME: handle different voxel types
        make_cube(voxel, info, lpos, vis, entropy);
    }
}

void GL_MeshingTask::finalize(void)
{
    if(!globals::dimension || !globals::dimension->chunks.valid(m_entity)) {
        // We either disconnected or something
        // else happened that invalidated the entity
        return;
    }

    auto& component = globals::dimension->chunks.emplace_or_replace<world::ChunkMesh>(m_entity);

    const std::size_t plane_count_nb = m_quads_s.size();
    const std::size_t plane_count_b = m_quads_b.size();

    bool has_no_submeshes_b = true;
    bool has_no_submeshes_nb = true;

    component.quad_nb.resize(plane_count_nb);
    component.quad_b.resize(plane_count_b);

    for(std::size_t plane = 0; plane < plane_count_nb; ++plane) {
        auto& builder = m_quads_s[plane];
        auto& buffer = component.quad_nb[plane];

        if(builder.empty()) {
            if(buffer.handle) {
                glDeleteBuffers(1, &buffer.handle);
                buffer.handle = 0;
                buffer.size = 0;
            }
        }
        else {
            if(!buffer.handle) {
                glGenBuffers(1, &buffer.handle);
            }

            glBindBuffer(GL_ARRAY_BUFFER, buffer.handle);
            glBufferData(GL_ARRAY_BUFFER, sizeof(world::ChunkQuad) * builder.size(), builder.data(), GL_STATIC_DRAW);
            buffer.size = builder.size();
            has_no_submeshes_nb = false;
        }
    }

    for(std::size_t plane = 0; plane < plane_count_b; ++plane) {
        auto& builder = m_quads_b[plane];
        auto& buffer = component.quad_b[plane];

        if(builder.empty()) {
            if(buffer.handle) {
                glDeleteBuffers(1, &buffer.handle);
                buffer.handle = 0;
                buffer.size = 0;
            }
        }
        else {
            if(!buffer.handle) {
                glGenBuffers(1, &buffer.handle);
            }

            glBindBuffer(GL_ARRAY_BUFFER, buffer.handle);
            glBufferData(GL_ARRAY_BUFFER, sizeof(world::ChunkQuad) * builder.size(), builder.data(), GL_STATIC_DRAW);
            buffer.size = builder.size();
            has_no_submeshes_b = false;
        }
    }

    if(has_no_submeshes_b && has_no_submeshes_nb) {
        globals::dimension->chunks.remove<world::ChunkMesh>(m_entity);
    }
}

bool GL_MeshingTask::vis_test(voxel_id voxel, const world::VoxelInfo* info, const local_pos& lpos) const
{
    const auto pvpos = coord::to_voxel(m_cpos, lpos);
    const auto pcpos = coord::to_chunk(pvpos);
    const auto plpos = coord::to_local(pvpos);
    const auto index = coord::to_index(plpos);

    const auto cached_cpos = get_cached_cpos(m_cpos, pcpos);
    const auto& voxels = m_cache.at(cached_cpos);
    const auto neighbour = voxels[index];

    bool result;

    if(neighbour == NULL_VOXEL_ID) {
        result = true;
    }
    else if(neighbour == voxel) {
        result = false;
    }
    else if(auto neighbour_info = world::voxel_registry::find(neighbour)) {
        if(neighbour_info->blending != info->blending) {
            // Voxel types that use blending are semi-transparent;
            // this means they're rendered using a different setup
            // and they must have visible faces with opaque voxels
            result = neighbour_info->blending;
        }
        else {
            result = false;
        }
    }
    else {
        result = false;
    }

    return result;
}

void GL_MeshingTask::push_quad_a(const world::VoxelInfo* info, const glm::fvec3& pos, const glm::fvec2& size, world::voxel_face face)
{
    const world::voxel_facing facing = get_facing(face, info->type);
    const world::VoxelTexture& vtex = info->textures[static_cast<std::size_t>(face)];

    if(info->blending) {
        m_quads_b[vtex.cached_plane].push_back(make_chunk_quad(pos, size, facing, vtex.cached_offset, vtex.paths.size()));
    }
    else {
        m_quads_s[vtex.cached_plane].push_back(make_chunk_quad(pos, size, facing, vtex.cached_offset, vtex.paths.size()));
    }
}

void GL_MeshingTask::push_quad_v(const world::VoxelInfo* info, const glm::fvec3& pos, const glm::fvec2& size, world::voxel_face face,
        std::size_t entropy)
{
    const world::voxel_facing facing = get_facing(face, info->type);
    const world::VoxelTexture& vtex = info->textures[static_cast<std::size_t>(face)];
    const std::size_t entropy_mod = entropy % vtex.paths.size();

    if(info->blending) {
        m_quads_b[vtex.cached_plane].push_back(make_chunk_quad(pos, size, facing, vtex.cached_offset + entropy_mod, 0));
    }
    else {
        m_quads_s[vtex.cached_plane].push_back(make_chunk_quad(pos, size, facing, vtex.cached_offset + entropy_mod, 0));
    }
}

void GL_MeshingTask::make_cube(voxel_id voxel, const world::VoxelInfo* info, const local_pos& lpos, world::voxel_vis vis,
        std::size_t entropy)
{
    const glm::fvec3 fpos = glm::fvec3(lpos);
    const glm::fvec2 fsize = glm::fvec2(1.0f, 1.0f);

    if(info->animated) {
        if(vis & world::VIS_NORTH) {
            push_quad_a(info, fpos, fsize, world::voxel_face::CUBE_NORTH);
        }

        if(vis & world::VIS_SOUTH) {
            push_quad_a(info, fpos, fsize, world::voxel_face::CUBE_SOUTH);
        }

        if(vis & world::VIS_EAST) {
            push_quad_a(info, fpos, fsize, world::voxel_face::CUBE_EAST);
        }

        if(vis & world::VIS_WEST) {
            push_quad_a(info, fpos, fsize, world::voxel_face::CUBE_WEST);
        }

        if(vis & world::VIS_UP) {
            push_quad_a(info, fpos, fsize, world::voxel_face::CUBE_TOP);
        }

        if(vis & world::VIS_DOWN) {
            push_quad_a(info, fpos, fsize, world::voxel_face::CUBE_BOTTOM);
        }
    }
    else {
        if(vis & world::VIS_NORTH) {
            push_quad_v(info, fpos, fsize, world::voxel_face::CUBE_NORTH, entropy);
        }

        if(vis & world::VIS_SOUTH) {
            push_quad_v(info, fpos, fsize, world::voxel_face::CUBE_SOUTH, entropy);
        }

        if(vis & world::VIS_EAST) {
            push_quad_v(info, fpos, fsize, world::voxel_face::CUBE_EAST, entropy);
        }

        if(vis & world::VIS_WEST) {
            push_quad_v(info, fpos, fsize, world::voxel_face::CUBE_WEST, entropy);
        }

        if(vis & world::VIS_UP) {
            push_quad_v(info, fpos, fsize, world::voxel_face::CUBE_TOP, entropy);
        }

        if(vis & world::VIS_DOWN) {
            push_quad_v(info, fpos, fsize, world::voxel_face::CUBE_BOTTOM, entropy);
        }
    }
}

void GL_MeshingTask::cache_chunk(const chunk_pos& cpos)
{
    const auto index = get_cached_cpos(m_cpos, cpos);

    if(const auto chunk = globals::dimension->find_chunk(cpos)) {
        m_cache[index] = chunk->get_voxels();
        return;
    }
}

// Bogus internal flag component
struct NeedsMeshingComponent final {};

static void on_chunk_create(const world::ChunkCreateEvent& event)
{
    const std::array<chunk_pos, 6> neighbours = {
        event.cpos + DIR_NORTH<chunk_pos::value_type>,
        event.cpos + DIR_SOUTH<chunk_pos::value_type>,
        event.cpos + DIR_EAST<chunk_pos::value_type>,
        event.cpos + DIR_WEST<chunk_pos::value_type>,
        event.cpos + DIR_UP<chunk_pos::value_type>,
        event.cpos + DIR_DOWN<chunk_pos::value_type>,
    };

    globals::dimension->chunks.emplace_or_replace<NeedsMeshingComponent>(event.chunk->get_entity());

    for(const chunk_pos& cpos : neighbours) {
        if(const world::Chunk* chunk = globals::dimension->find_chunk(cpos)) {
            globals::dimension->chunks.emplace_or_replace<NeedsMeshingComponent>(chunk->get_entity());
            continue;
        }
    }
}

static void on_chunk_update(const world::ChunkUpdateEvent& event)
{
    const std::array<chunk_pos, 6> neighbours = {
        event.cpos + DIR_NORTH<chunk_pos::value_type>,
        event.cpos + DIR_SOUTH<chunk_pos::value_type>,
        event.cpos + DIR_EAST<chunk_pos::value_type>,
        event.cpos + DIR_WEST<chunk_pos::value_type>,
        event.cpos + DIR_UP<chunk_pos::value_type>,
        event.cpos + DIR_DOWN<chunk_pos::value_type>,
    };

    globals::dimension->chunks.emplace_or_replace<NeedsMeshingComponent>(event.chunk->get_entity());

    for(const chunk_pos& cpos : neighbours) {
        if(const world::Chunk* chunk = globals::dimension->find_chunk(cpos)) {
            globals::dimension->chunks.emplace_or_replace<NeedsMeshingComponent>(chunk->get_entity());
            continue;
        }
    }
}

static void on_voxel_set(const world::VoxelSetEvent& event)
{
    globals::dimension->chunks.emplace_or_replace<NeedsMeshingComponent>(event.chunk->get_entity());

    std::vector<chunk_pos> neighbours;

    for(int dim = 0; dim < 3; dim += 1) {
        chunk_pos offset = chunk_pos(0, 0, 0);
        offset[dim] = 1;

        if(event.lpos[dim] == 0) {
            neighbours.push_back(event.cpos - offset);
            continue;
        }

        if(event.lpos[dim] == (CHUNK_SIZE - 1)) {
            neighbours.push_back(event.cpos + offset);
            continue;
        }
    }

    for(const chunk_pos& cpos : neighbours) {
        if(const world::Chunk* chunk = globals::dimension->find_chunk(cpos)) {
            globals::dimension->chunks.emplace_or_replace<NeedsMeshingComponent>(chunk->get_entity());
            continue;
        }
    }
}

void world::chunk_mesher::init(void)
{
    globals::dispatcher.sink<ChunkCreateEvent>().connect<&on_chunk_create>();
    globals::dispatcher.sink<ChunkUpdateEvent>().connect<&on_chunk_update>();
    globals::dispatcher.sink<VoxelSetEvent>().connect<&on_voxel_set>();
}

void world::chunk_mesher::shutdown(void)
{
}

void world::chunk_mesher::update(void)
{
    if(session::is_ingame()) {
        const auto group = globals::dimension->chunks.group<NeedsMeshingComponent>(entt::get<ChunkComponent>);
        for(const auto [entity, chunk] : group.each()) {
            globals::dimension->chunks.remove<NeedsMeshingComponent>(entity);
            threading::submit<GL_MeshingTask>(entity, chunk.cpos);
        }
    }
}
