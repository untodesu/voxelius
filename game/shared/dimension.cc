#include "shared/pch.hh"

#include "shared/dimension.hh"

#include "shared/chunk.hh"
#include "shared/coord.hh"
#include "shared/globals.hh"

Dimension::Dimension(const char* name, float gravity)
{
    m_name = name;
    m_gravity = gravity;
}

Dimension::~Dimension(void)
{
    for(const auto it : m_chunkmap)
        delete it.second;
    entities.clear();
    chunks.clear();
}

const char* Dimension::get_name(void) const
{
    return m_name.c_str();
}

float Dimension::get_gravity(void) const
{
    return m_gravity;
}

Chunk* Dimension::create_chunk(const chunk_pos& cpos)
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

Chunk* Dimension::find_chunk(entt::entity entity) const
{
    if(chunks.valid(entity)) {
        return chunks.get<ChunkComponent>(entity).chunk;
    } else {
        return nullptr;
    }
}

Chunk* Dimension::find_chunk(const chunk_pos& cpos) const
{
    auto it = m_chunkmap.find(cpos);

    if(it != m_chunkmap.cend()) {
        return it->second;
    } else {
        return nullptr;
    }
}

void Dimension::remove_chunk(entt::entity entity)
{
    if(chunks.valid(entity)) {
        auto& component = chunks.get<ChunkComponent>(entity);
        m_chunkmap.erase(component.cpos);
        chunks.destroy(entity);
    }
}

void Dimension::remove_chunk(const chunk_pos& cpos)
{
    auto it = m_chunkmap.find(cpos);

    if(it != m_chunkmap.cend()) {
        chunks.destroy(it->second->get_entity());
        m_chunkmap.erase(it);
    }
}

void Dimension::remove_chunk(Chunk* chunk)
{
    if(chunk) {
        const auto& component = chunks.get<ChunkComponent>(chunk->get_entity());
        m_chunkmap.erase(component.cpos);
        chunks.destroy(chunk->get_entity());
    }
}

voxel_id Dimension::get_voxel(const voxel_pos& vpos) const
{
    auto cpos = coord::to_chunk(vpos);
    auto lpos = coord::to_local(vpos);

    if(auto chunk = find_chunk(cpos)) {
        return chunk->get_voxel(lpos);
    } else {
        return NULL_VOXEL_ID;
    }
}

voxel_id Dimension::get_voxel(const chunk_pos& cpos, const local_pos& lpos) const
{
    // This allows accessing get_voxel with negative
    // local coordinates that usually would result in an
    // out-of-range values; this is useful for per-voxel update logic
    return get_voxel(coord::to_voxel(cpos, lpos));
}

bool Dimension::set_voxel(voxel_id voxel, const voxel_pos& vpos)
{
    auto cpos = coord::to_chunk(vpos);
    auto lpos = coord::to_local(vpos);

    if(auto chunk = find_chunk(cpos)) {
        chunk->set_voxel(voxel, lpos);

        VoxelSetEvent event;
        event.dimension = this;
        event.cpos = cpos;
        event.lpos = lpos;
        event.voxel = voxel;
        event.chunk = chunk;

        globals::dispatcher.trigger(event);

        return true;
    }

    return false;
}

bool Dimension::set_voxel(voxel_id voxel, const chunk_pos& cpos, const local_pos& lpos)
{
    // This allows accessing set_voxel with negative
    // local coordinates that usually would result in an
    // out-of-range values; this is useful for per-voxel update logic
    return set_voxel(voxel, coord::to_voxel(cpos, lpos));
}

void Dimension::init(Config& config)
{
}

void Dimension::init_late(std::uint64_t global_seed)
{
}

bool Dimension::generate(const chunk_pos& cpos, VoxelStorage& voxels)
{
    return false;
}
