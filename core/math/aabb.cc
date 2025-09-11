#include "core/pch.hh"

#include "core/math/aabb.hh"

math::AABB::AABB(const glm::fvec3& min, const glm::fvec3& max)
{
    set_bounds(min, max);
}

void math::AABB::set_bounds(const glm::fvec3& min, const glm::fvec3& max)
{
    this->min = min;
    this->max = max;
}

void math::AABB::set_offset(const glm::fvec3& base, const glm::fvec3& size)
{
    this->min = base;
    this->max = base + size;
}

bool math::AABB::contains(const glm::fvec3& point) const
{
    auto result = true;
    result = result && (point.x >= min.x) && (point.x <= max.x);
    result = result && (point.y >= min.y) && (point.y <= max.y);
    result = result && (point.z >= min.z) && (point.z <= max.z);
    return result;
}

bool math::AABB::intersect(const AABB& other_box) const
{
    auto result = true;
    result = result && (min.x < other_box.max.x) && (max.x > other_box.min.x);
    result = result && (min.y < other_box.max.y) && (max.y > other_box.min.y);
    result = result && (min.z < other_box.max.z) && (max.z > other_box.min.z);
    return result;
}

math::AABB math::AABB::combine_with(const math::AABB& other_box) const
{
    AABB result;
    result.set_bounds(min, other_box.max);
    return result;
}

math::AABB math::AABB::multiply_with(const math::AABB& other_box) const
{
    AABB result;
    result.set_bounds(other_box.min, max);
    return result;
}

math::AABB math::AABB::push(const glm::fvec3& vector) const
{
    AABB result;
    result.set_bounds(min + vector, max + vector);
    return result;
}
