#ifndef SHARED_CHUNK_HH
#define SHARED_CHUNK_HH 1
#pragma once

#include "shared/types.hh"
#include "shared/voxel_storage.hh"

constexpr static unsigned int BIOME_VOID = 0U;

class Dimension;

class Chunk final {
public:
    explicit Chunk(entt::entity entity, Dimension *dimension);
    virtual ~Chunk(void) = default;

    voxel_id get_voxel(const local_pos &lpos) const;
    voxel_id get_voxel(const std::size_t index) const;

    void set_voxel(voxel_id voxel, const local_pos &lpos);
    void set_voxel(voxel_id voxel, const std::size_t index);

    const VoxelStorage &get_voxels(void) const;
    void set_voxels(const VoxelStorage &voxels);

    unsigned int get_biome(void) const;
    void set_biome(unsigned int biome);

    entt::entity get_entity(void) const;
    Dimension *get_dimension(void) const;

private:
    entt::entity m_entity;
    Dimension *m_dimension;
    VoxelStorage m_voxels;
    unsigned int m_biome;
};

#endif /* SHARED_CHUNK_HH */
