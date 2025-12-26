// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: ray_dda.hh
// Description: Fast voxel grid traversal with an arbitrary starting position

#ifndef SHARED_WORLD_RAY_DDA_HH
#define SHARED_WORLD_RAY_DDA_HH
#pragma once

#include "shared/types.hh"

class Dimension;
class Voxel;

class RayDDA final {
public:
    RayDDA(void) = default;
    explicit RayDDA(const Dimension* dimension, const chunk_pos& start_chunk, const glm::fvec3& start_fpos, const glm::fvec3& direction);
    explicit RayDDA(const Dimension& dimension, const chunk_pos& start_chunk, const glm::fvec3& start_fpos, const glm::fvec3& direction);

    void reset(const Dimension* dimension, const chunk_pos& start_chunk, const glm::fvec3& start_fpos, const glm::fvec3& direction);
    void reset(const Dimension& dimension, const chunk_pos& start_chunk, const glm::fvec3& start_fpos, const glm::fvec3& direction);

    const Voxel* step(void);

public:
    const Dimension* dimension;
    chunk_pos start_chunk;
    glm::fvec3 start_fpos;
    glm::fvec3 direction;

    glm::fvec3 delta_dist;
    glm::fvec3 side_dist;
    voxel_pos vstep;

    double distance;
    voxel_pos vnormal;
    voxel_pos vpos;
};

#endif
