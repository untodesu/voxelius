#include "core/pch.hh"
#include "core/aabb.hh"

AABB::AABB(const glm::fvec3 &min, const glm::fvec3 &max)
{
    set_bounds(min, max);
}

void AABB::set_bounds(const glm::fvec3 &min, const glm::fvec3 &max)
{
    this->min = min;
    this->max = max;
}

void AABB::set_offset(const glm::fvec3 &base, const glm::fvec3 &size)
{
    this->min = base;
    this->max = base + size;
}

bool AABB::contains(const glm::fvec3 &point) const
{
    if((point.x < min.x) || (point.x > max.x))
        return false;
    if((point.y < min.y) || (point.y > max.y))
        return false;
    if((point.z < min.z) || (point.z > max.z))
        return false;
    return true;
}

bool AABB::intersect(const AABB &other_box) const
{
    if((min.x >= other_box.max.x) || (max.x <= other_box.min.x))
        return false;
    if((min.y >= other_box.max.y) || (max.y <= other_box.min.y))
        return false;
    if((min.z >= other_box.max.z) || (max.z <= other_box.min.z))
        return false;
    return true;
}

AABB AABB::combine_with(const AABB &other_box) const
{
    AABB result;
    result.set_bounds(min, other_box.max);
    return result;
}

AABB AABB::multiply_with(const AABB &other_box) const
{
    AABB result;
    result.set_bounds(other_box.min, max);
    return result;
}

AABB AABB::push(const glm::fvec3 &vector) const
{
    AABB result;
    result.set_bounds(min + vector, max + vector);
    return result;
}
