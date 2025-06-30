#include "shared/pch.hh"

#include "shared/world/chunk_aabb.hh"

void world::ChunkAABB::set_bounds(const chunk_pos& min, const chunk_pos& max)
{
    this->min = min;
    this->max = max;
}

void world::ChunkAABB::set_offset(const chunk_pos& base, const chunk_pos& size)
{
    this->min = base;
    this->max = base + size;
}

bool world::ChunkAABB::contains(const chunk_pos& point) const
{
    auto result = true;
    result = result && (point.x >= min.x) && (point.x <= max.x);
    result = result && (point.y >= min.y) && (point.y <= max.y);
    result = result && (point.z >= min.z) && (point.z <= max.z);
    return result;
}

bool world::ChunkAABB::intersect(const ChunkAABB& other_box) const
{
    auto result = true;
    result = result && (min.x < other_box.max.x) && (max.x > other_box.min.x);
    result = result && (min.y < other_box.max.y) && (max.y > other_box.min.y);
    result = result && (min.z < other_box.max.z) && (max.z > other_box.min.z);
    return result;
}

world::ChunkAABB world::ChunkAABB::combine_with(const ChunkAABB& other_box) const
{
    ChunkAABB result;
    result.set_bounds(min, other_box.max);
    return result;
}

world::ChunkAABB world::ChunkAABB::multiply_with(const ChunkAABB& other_box) const
{
    ChunkAABB result;
    result.set_bounds(other_box.min, max);
    return result;
}

world::ChunkAABB world::ChunkAABB::push(const chunk_pos& vector) const
{
    ChunkAABB result;
    result.set_bounds(min + vector, max + vector);
    return result;
}
