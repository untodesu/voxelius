#pragma once

#include "shared/types.hh"

namespace world
{
class Dimension;
class Voxel;
} // namespace world

namespace world
{
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
} // namespace world
