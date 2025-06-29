#ifndef SHARED_FEATURE_HH
#define SHARED_FEATURE_HH 1
#pragma once

#include "core/macros.hh"

#include "shared/types.hh"

class Dimension;
class VoxelStorage;

class Feature final : public std::vector<std::tuple<voxel_pos, voxel_id, bool>> {
public:
    DECLARE_DEFAULT_CONSTRUCTOR(Feature);
    virtual ~Feature(void) = default;

public:
    void place(const voxel_pos& vpos, Dimension* dimension) const;
    void place(const voxel_pos& vpos, const chunk_pos& cpos, VoxelStorage& voxels) const;
};

#endif /* SHARED_FEATURE_HH */
