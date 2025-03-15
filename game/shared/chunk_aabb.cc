#include "shared/pch.hh"
#include "shared/chunk_aabb.hh"

void ChunkAABB::set_bounds(const chunk_pos &min, const chunk_pos &max)
{
    this->min = min;
    this->max = max;
}

void ChunkAABB::set_offset(const chunk_pos &base, const chunk_pos &size)
{
    this->min = base;
    this->max = base + size;
}

bool ChunkAABB::contains(const chunk_pos &point) const
{
    if((point.x < min.x) || (point.x > max.x))
        return false;
    if((point.y < min.y) || (point.y > max.y))
        return false;
    if((point.z < min.z) || (point.z > max.z))
        return false;
    return true;
}

bool ChunkAABB::intersect(const ChunkAABB &other_box) const
{
    if((min.x >= other_box.max.x) || (max.x <= other_box.min.x))
        return false;
    if((min.y >= other_box.max.y) || (max.y <= other_box.min.y))
        return false;
    if((min.z >= other_box.max.z) || (max.z <= other_box.min.z))
        return false;
    return true;
}

ChunkAABB ChunkAABB::combine_with(const ChunkAABB &other_box) const
{
    ChunkAABB result;
    result.set_bounds(min, other_box.max);
    return result;
}

ChunkAABB ChunkAABB::multiply_with(const ChunkAABB &other_box) const
{
    ChunkAABB result;
    result.set_bounds(other_box.min, max);
    return result;
}

ChunkAABB ChunkAABB::push(const chunk_pos &vector) const
{
    ChunkAABB result;
    result.set_bounds(min + vector, max + vector);
    return result;
}
