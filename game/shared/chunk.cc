#include "shared/pch.hh"
#include "shared/chunk.hh"

#include "shared/coord.hh"

Chunk::Chunk(entt::entity entity, Dimension *dimension)
{
    m_entity = entity;
    m_dimension = dimension;
    m_voxels.fill(NULL_VOXEL_ID);
}

voxel_id Chunk::get_voxel(const local_pos &lpos) const
{
    return get_voxel(coord::to_index(lpos));
}

voxel_id Chunk::get_voxel(const std::size_t index) const
{
    if(index >= CHUNK_VOLUME)
        return NULL_VOXEL_ID;
    return m_voxels[index];
}

void Chunk::set_voxel(voxel_id voxel, const local_pos &lpos)
{
    set_voxel(voxel, coord::to_index(lpos));
}

void Chunk::set_voxel(voxel_id voxel, const std::size_t index)
{
    if(index >= CHUNK_VOLUME)
        return;
    m_voxels[index] = voxel;
}

const VoxelStorage &Chunk::get_voxels(void) const
{
    return m_voxels;
}

void Chunk::set_voxels(const VoxelStorage &voxels)
{
    m_voxels = voxels;
}

entt::entity Chunk::get_entity(void) const
{
    return m_entity;
}

Dimension *Chunk::get_dimension(void) const
{
    return m_dimension;
}
