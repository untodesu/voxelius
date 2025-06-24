#ifndef SHARED_RAY_DDA
#define SHARED_RAY_DDA 1
#pragma once

#include "shared/types.hh"

class Dimension;

class RayDDA final {
public:
    explicit RayDDA(void) = default;
    explicit RayDDA(const Dimension* dimension, const chunk_pos& start_chunk, const glm::fvec3& start_fpos, const glm::fvec3& direction);
    explicit RayDDA(const Dimension& dimension, const chunk_pos& start_chunk, const glm::fvec3& start_fpos, const glm::fvec3& direction);

    void reset(const Dimension* dimension, const chunk_pos& start_chunk, const glm::fvec3& start_fpos, const glm::fvec3& direction);
    void reset(const Dimension& dimension, const chunk_pos& start_chunk, const glm::fvec3& start_fpos, const glm::fvec3& direction);

    voxel_id step(void);

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

#endif /* SHARED_RAY_DDA */
