// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: feature.hh
// Description: A world generation feature

#ifndef SHARED_WORLD_FEATURE_HH
#define SHARED_WORLD_FEATURE_HH
#pragma once

#include "shared/types.hh"

class Chunk;
class Dimension;
class Voxel;
class VoxelStorage;

class Feature final : public std::vector<std::tuple<voxel_pos, const Voxel*, bool>> {
public:
    Feature(void) = default;
    virtual ~Feature(void) = default;

public:
    void place(const voxel_pos& vpos, Dimension* dimension) const;
    void place(const voxel_pos& vpos, const chunk_pos& cpos, Chunk& chunk) const;
    void place(const voxel_pos& vpos, const chunk_pos& cpos, VoxelStorage& voxels) const;
};

#endif
