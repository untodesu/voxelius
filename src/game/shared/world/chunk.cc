// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: chunk.cc
// Description: A single chunk of voxels

#include "shared/pch.hh"

#include "shared/world/chunk.hh"

#include "shared/world/voxel_registry.hh"

#include "shared/coord.hh"

Chunk::Chunk(entt::entity entity, Dimension* dimension)
{
    m_entity = entity;
    m_dimension = dimension;
    m_voxels.fill(NULL_VOXEL_ID);
    m_biome = BIOME_VOID;
}

const Voxel* Chunk::get_voxel(const local_pos& lpos) const
{
    return get_voxel(coord::to_index(lpos));
}

const Voxel* Chunk::get_voxel(const std::size_t index) const
{
    if(index >= CHUNK_VOLUME) {
        return nullptr;
    }

    return voxel_registry::find(m_voxels[index]);
}

void Chunk::set_voxel(const Voxel* voxel, const local_pos& lpos)
{
    set_voxel(voxel, coord::to_index(lpos));
}

void Chunk::set_voxel(const Voxel* voxel, const std::size_t index)
{
    if(index < CHUNK_VOLUME) {
        m_voxels[index] = voxel ? voxel->get_id() : NULL_VOXEL_ID;
        return;
    }
}

const VoxelStorage& Chunk::get_voxels(void) const
{
    return m_voxels;
}

void Chunk::set_voxels(const VoxelStorage& voxels)
{
    m_voxels = voxels;
}

unsigned int Chunk::get_biome(void) const
{
    return m_biome;
}

void Chunk::set_biome(unsigned int biome)
{
    m_biome = biome;
}

entt::entity Chunk::get_entity(void) const
{
    return m_entity;
}

Dimension* Chunk::get_dimension(void) const
{
    return m_dimension;
}
