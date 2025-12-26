#include "shared/pch.hh"

#include "shared/world/ray_aabb.hh"

RayAABB::RayAABB(const glm::fvec3& start, const glm::fvec3& dir) noexcept
{
    reset(start, dir);
}

void RayAABB::reset(const glm::fvec3& start, const glm::fvec3& dir) noexcept
{
    assert(std::isfinite(start.x));
    assert(std::isfinite(start.y));
    assert(std::isfinite(start.z));
    assert(std::isfinite(dir.x));
    assert(std::isfinite(dir.y));
    assert(std::isfinite(dir.z));

    m_start_pos = start;
    m_direction = dir;
}

bool RayAABB::intersect(const math::AABBf& aabb, float& distance, glm::fvec3& surface) const noexcept
{
    // Adapted from https://github.com/gdbooks/3DCollisions/blob/master/Chapter3/raycast_aabb.md

    constexpr static std::array<glm::fvec3, 6> TNORMALS = {
        glm::fvec3(-1.0f, 0.0f, 0.0f),
        glm::fvec3(1.0f, 0.0f, 0.0f),
        glm::fvec3(0.0f, -1.0f, 0.0f),
        glm::fvec3(0.0f, 1.0f, 0.0f),
        glm::fvec3(0.0f, 0.0f, -1.0f),
        glm::fvec3(0.0f, 0.0f, 1.0f),
    };

    std::array<float, 6> tvalues;
    tvalues[0] = (aabb.min.x - m_start_pos.x) / m_direction.x;
    tvalues[1] = (aabb.max.x - m_start_pos.x) / m_direction.x;
    tvalues[2] = (aabb.min.y - m_start_pos.y) / m_direction.y;
    tvalues[3] = (aabb.max.y - m_start_pos.y) / m_direction.y;
    tvalues[4] = (aabb.min.z - m_start_pos.z) / m_direction.z;
    tvalues[5] = (aabb.max.z - m_start_pos.z) / m_direction.z;

    static_assert(tvalues.size() == TNORMALS.size());

    auto tmin = std::max(std::max(std::min(tvalues[0], tvalues[1]), std::min(tvalues[2], tvalues[3])), std::min(tvalues[4], tvalues[5]));
    auto tmax = std::min(std::min(std::max(tvalues[0], tvalues[1]), std::max(tvalues[2], tvalues[3])), std::max(tvalues[4], tvalues[5]));

    if(tmax < 0.0f || tmin > tmax) {
        distance = std::numeric_limits<float>::quiet_NaN();
        return false; // no intersection
    }

    for(std::size_t i = 0U; i < tvalues.size(); ++i) {
        if(tvalues[i] == tmin) {
            surface = TNORMALS[i];
            break;
        }
    }

    distance = tmin < 0.0f ? tmax : tmin;

    return true;
}
