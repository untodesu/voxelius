#ifndef SHARED_FEATURE_HH
#define SHARED_FEATURE_HH 1
#pragma once

#include "shared/types.hh"

class Dimension;
class VoxelStorage;

class Feature final : public std::vector<std::pair<voxel_pos, voxel_id>> {
public:
    explicit Feature(void) = default;
    virtual ~Feature(void) = default;

public:
    void place(const voxel_pos &vpos, Dimension *dimension, bool overwrite = false) const;
    void place(const voxel_pos &vpos, const chunk_pos &cpos, VoxelStorage &voxels, bool overwrite = false) const;
};

#endif /* SHARED_FEATURE_HH */
