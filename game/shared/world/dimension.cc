#include "shared/pch.hh"

#include "shared/world/dimension.hh"

#include "shared/world/chunk.hh"
#include "shared/world/voxel_registry.hh"

#include "shared/coord.hh"
#include "shared/globals.hh"

world::Dimension::Dimension(std::string_view name, float gravity)
{
    m_name = name;
    m_gravity = gravity;
}

world::Dimension::~Dimension(void)
{
    for(const auto it : m_chunkmap)
        delete it.second;
    entities.clear();
    chunks.clear();
}

std::string_view world::Dimension::get_name(void) const
{
    return m_name;
}

float world::Dimension::get_gravity(void) const
{
    return m_gravity;
}

world::Chunk* world::Dimension::create_chunk(const chunk_pos& cpos)
{
    auto it = m_chunkmap.find(cpos);

    if(it != m_chunkmap.cend()) {
        // Chunk already exists
        return it->second;
    }

    auto entity = chunks.create();
    auto chunk = new Chunk(entity, this);

    auto& component = chunks.emplace<ChunkComponent>(entity);
    component.chunk = chunk;
    component.cpos = cpos;

    ChunkCreateEvent event;
    event.dimension = this;
    event.chunk = chunk;
    event.cpos = cpos;

    globals::dispatcher.trigger(event);

    return m_chunkmap.insert_or_assign(cpos, std::move(chunk)).first->second;
}

world::Chunk* world::Dimension::find_chunk(entt::entity entity) const
{
    if(chunks.valid(entity)) {
        return chunks.get<ChunkComponent>(entity).chunk;
    }
    else {
        return nullptr;
    }
}

world::Chunk* world::Dimension::find_chunk(const chunk_pos& cpos) const
{
    auto it = m_chunkmap.find(cpos);

    if(it != m_chunkmap.cend()) {
        return it->second;
    }
    else {
        return nullptr;
    }
}

void world::Dimension::remove_chunk(entt::entity entity)
{
    if(chunks.valid(entity)) {
        auto& component = chunks.get<ChunkComponent>(entity);
        m_chunkmap.erase(component.cpos);
        chunks.destroy(entity);
    }
}

void world::Dimension::remove_chunk(const chunk_pos& cpos)
{
    auto it = m_chunkmap.find(cpos);

    if(it != m_chunkmap.cend()) {
        chunks.destroy(it->second->get_entity());
        m_chunkmap.erase(it);
    }
}

void world::Dimension::remove_chunk(Chunk* chunk)
{
    if(chunk) {
        const auto& component = chunks.get<ChunkComponent>(chunk->get_entity());
        m_chunkmap.erase(component.cpos);
        chunks.destroy(chunk->get_entity());
    }
}

const world::Voxel* world::Dimension::get_voxel(const voxel_pos& vpos) const
{
    auto cpos = coord::to_chunk(vpos);
    auto lpos = coord::to_local(vpos);

    if(auto chunk = find_chunk(cpos)) {
        return chunk->get_voxel(lpos);
    }

    return nullptr;
}

const world::Voxel* world::Dimension::get_voxel(const chunk_pos& cpos, const local_pos& lpos) const
{
    // This allows accessing get_voxel with negative
    // local coordinates that usually would result in an
    // out-of-range values; this is useful for per-voxel update logic
    return get_voxel(coord::to_voxel(cpos, lpos));
}

bool world::Dimension::set_voxel(const Voxel* voxel, const voxel_pos& vpos)
{
    auto cpos = coord::to_chunk(vpos);
    auto lpos = coord::to_local(vpos);

    if(auto chunk = find_chunk(cpos)) {
        if(auto old_voxel = chunk->get_voxel(lpos)) {
            if(old_voxel != voxel) {
                // Notify the old voxel that it is
                // being replaced with a different voxel
                old_voxel->on_remove(this, vpos);
            }
        }

        chunk->set_voxel(voxel, lpos);

        if(voxel) {
            // If we're not placing air, notify the
            // new voxel that it has been placed
            voxel->on_place(this, vpos);
        }

        VoxelSetEvent event;
        event.dimension = this;
        event.voxel = voxel;
        event.cpos = cpos;
        event.lpos = lpos;
        event.chunk = chunk;

        globals::dispatcher.trigger(event);

        return true;
    }

    return false;
}

bool world::Dimension::set_voxel(const Voxel* voxel, const chunk_pos& cpos, const local_pos& lpos)
{
    // This allows accessing set_voxel with negative
    // local coordinates that usually would result in an
    // out-of-range values; this is useful for per-voxel update logic
    return set_voxel(voxel, coord::to_voxel(cpos, lpos));
}

void world::Dimension::init(io::ConfigMap& config)
{
}

void world::Dimension::init_late(std::uint64_t global_seed)
{
}

bool world::Dimension::generate(const chunk_pos& cpos, VoxelStorage& voxels)
{
    return false;
}
