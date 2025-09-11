#pragma once

#include "shared/world/voxel_storage.hh"

#include "shared/types.hh"

constexpr static unsigned int BIOME_VOID = 0U;

namespace world
{
class Dimension;
class Voxel;
} // namespace world

namespace world
{
class Chunk final {
public:
    explicit Chunk(entt::entity entity, Dimension* dimension);
    virtual ~Chunk(void) = default;

    const Voxel* get_voxel(const local_pos& lpos) const;
    const Voxel* get_voxel(const std::size_t index) const;

    void set_voxel(const Voxel* voxel, const local_pos& lpos);
    void set_voxel(const Voxel* voxel, const std::size_t index);

    const VoxelStorage& get_voxels(void) const;
    void set_voxels(const VoxelStorage& voxels);

    unsigned int get_biome(void) const;
    void set_biome(unsigned int biome);

    entt::entity get_entity(void) const;
    Dimension* get_dimension(void) const;

private:
    entt::entity m_entity;
    Dimension* m_dimension;
    VoxelStorage m_voxels;
    unsigned int m_biome;
};
} // namespace world
