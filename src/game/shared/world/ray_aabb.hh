// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: ray_aabb.hh
// Description: Check ray intersections against an AABB

#ifndef SHARED_WORLD_RAY_AABB_HH
#define SHARED_WORLD_RAY_AABB_HH
#pragma once

#include "core/math/aabb.hh"

class RayAABB final {
public:
    RayAABB(void) = default;
    explicit RayAABB(const glm::fvec3& start, const glm::fvec3& dir) noexcept;

    constexpr const glm::fvec3& start_pos(void) const noexcept;
    constexpr const glm::fvec3& direction(void) const noexcept;

    void reset(const glm::fvec3& start, const glm::fvec3& dir) noexcept;

    bool intersect(const math::AABBf& aabb, float& distance, glm::fvec3& surface) const noexcept;

private:
    glm::fvec3 m_start_pos;
    glm::fvec3 m_direction;
};

constexpr const glm::fvec3& RayAABB::start_pos(void) const noexcept
{
    return m_start_pos;
}

constexpr const glm::fvec3& RayAABB::direction(void) const noexcept
{
    return m_direction;
}

#endif
