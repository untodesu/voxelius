#include "shared/pch.hh"

#include "shared/world/chunk.hh"

#include "shared/coord.hh"

world::Chunk::Chunk(entt::entity entity, Dimension* dimension)
{
    m_entity = entity;
    m_dimension = dimension;
    m_voxels.fill(NULL_VOXEL_ID);
    m_biome = BIOME_VOID;
}

voxel_id world::Chunk::get_voxel(const local_pos& lpos) const
{
    return get_voxel(coord::to_index(lpos));
}

voxel_id world::Chunk::get_voxel(const std::size_t index) const
{
    if(index >= CHUNK_VOLUME) {
        return NULL_VOXEL_ID;
    }
    else {
        return m_voxels[index];
    }
}

void world::Chunk::set_voxel(voxel_id voxel, const local_pos& lpos)
{
    set_voxel(voxel, coord::to_index(lpos));
}

void world::Chunk::set_voxel(voxel_id voxel, const std::size_t index)
{
    if(index < CHUNK_VOLUME) {
        m_voxels[index] = voxel;
    }
}

const world::VoxelStorage& world::Chunk::get_voxels(void) const
{
    return m_voxels;
}

void world::Chunk::set_voxels(const VoxelStorage& voxels)
{
    m_voxels = voxels;
}

unsigned int world::Chunk::get_biome(void) const
{
    return m_biome;
}

void world::Chunk::set_biome(unsigned int biome)
{
    m_biome = biome;
}

entt::entity world::Chunk::get_entity(void) const
{
    return m_entity;
}

world::Dimension* world::Chunk::get_dimension(void) const
{
    return m_dimension;
}
