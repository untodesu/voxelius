#pragma once

#include "shared/types.hh"

namespace world
{
class Dimension;
class VoxelStorage;
} // namespace world

namespace world
{
class Feature final : public std::vector<std::tuple<voxel_pos, voxel_id, bool>> {
public:
    Feature(void) = default;
    virtual ~Feature(void) = default;

public:
    void place(const voxel_pos& vpos, Dimension* dimension) const;
    void place(const voxel_pos& vpos, const chunk_pos& cpos, VoxelStorage& voxels) const;
};
} // namespace world
