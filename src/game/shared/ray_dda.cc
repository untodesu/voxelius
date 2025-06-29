#include "shared/pch.hh"

#include "shared/ray_dda.hh"

#include "shared/coord.hh"
#include "shared/dimension.hh"

RayDDA::RayDDA(const Dimension* dimension, const chunk_pos& start_chunk, const glm::fvec3& start_fpos, const glm::fvec3& direction)
{
    reset(dimension, start_chunk, start_fpos, direction);
}

RayDDA::RayDDA(const Dimension& dimension, const chunk_pos& start_chunk, const glm::fvec3& start_fpos, const glm::fvec3& direction)
{
    reset(dimension, start_chunk, start_fpos, direction);
}

void RayDDA::reset(const Dimension* dimension, const chunk_pos& start_chunk, const glm::fvec3& start_fpos, const glm::fvec3& direction)
{
    this->dimension = dimension;
    this->start_chunk = start_chunk;
    this->start_fpos = start_fpos;
    this->direction = direction;

    this->delta_dist.x = direction.x ? vx::abs(1.0f / direction.x) : std::numeric_limits<float>::max();
    this->delta_dist.y = direction.y ? vx::abs(1.0f / direction.y) : std::numeric_limits<float>::max();
    this->delta_dist.z = direction.z ? vx::abs(1.0f / direction.z) : std::numeric_limits<float>::max();

    this->distance = 0.0f;
    this->vpos = coord::to_voxel(start_chunk, start_fpos);
    this->vnormal = voxel_pos(0, 0, 0);

    // Need this for initial direction calculations
    auto lpos = coord::to_local(start_fpos);

    if(direction.x < 0.0f) {
        this->side_dist.x = this->delta_dist.x * (start_fpos.x - lpos.x);
        this->vstep.x = voxel_pos::value_type(-1);
    } else {
        this->side_dist.x = this->delta_dist.x * (lpos.x + 1.0f - start_fpos.x);
        this->vstep.x = voxel_pos::value_type(+1);
    }

    if(direction.y < 0.0f) {
        this->side_dist.y = this->delta_dist.y * (start_fpos.y - lpos.y);
        this->vstep.y = voxel_pos::value_type(-1);
    } else {
        this->side_dist.y = this->delta_dist.y * (lpos.y + 1.0f - start_fpos.y);
        this->vstep.y = voxel_pos::value_type(+1);
    }

    if(direction.z < 0.0f) {
        this->side_dist.z = this->delta_dist.z * (start_fpos.z - lpos.z);
        this->vstep.z = voxel_pos::value_type(-1);
    } else {
        this->side_dist.z = this->delta_dist.z * (lpos.z + 1.0f - start_fpos.z);
        this->vstep.z = voxel_pos::value_type(+1);
    }
}

void RayDDA::reset(const Dimension& dimension, const chunk_pos& start_chunk, const glm::fvec3& start_fpos, const glm::fvec3& direction)
{
    reset(&dimension, start_chunk, start_fpos, direction);
}

voxel_id RayDDA::step(void)
{
    if(side_dist.x < side_dist.z) {
        if(side_dist.x < side_dist.y) {
            vnormal = voxel_pos(-vstep.x, 0, 0);
            distance = side_dist.x;
            side_dist.x += delta_dist.x;
            vpos.x += vstep.x;
        } else {
            vnormal = voxel_pos(0, -vstep.y, 0);
            distance = side_dist.y;
            side_dist.y += delta_dist.y;
            vpos.y += vstep.y;
        }
    } else {
        if(side_dist.z < side_dist.y) {
            vnormal = voxel_pos(0, 0, -vstep.z);
            distance = side_dist.z;
            side_dist.z += delta_dist.z;
            vpos.z += vstep.z;
        } else {
            vnormal = voxel_pos(0, -vstep.y, 0);
            distance = side_dist.y;
            side_dist.y += delta_dist.y;
            vpos.y += vstep.y;
        }
    }

    // This is slower than I want it to be
    return dimension->get_voxel(vpos);
}
