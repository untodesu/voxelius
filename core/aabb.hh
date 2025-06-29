#ifndef CORE_AABB_HH
#define CORE_AABB_HH 1
#pragma once

#include "core/macros.hh"

class AABB final {
public:
    DECLARE_DEFAULT_CONSTRUCTOR(AABB);
    explicit AABB(const glm::fvec3& min, const glm::fvec3& max);
    virtual ~AABB(void) = default;

    void set_bounds(const glm::fvec3& min, const glm::fvec3& max);
    void set_offset(const glm::fvec3& base, const glm::fvec3& size);

    const glm::fvec3& get_min(void) const;
    const glm::fvec3& get_max(void) const;

    bool contains(const glm::fvec3& point) const;
    bool intersect(const AABB& other_box) const;

    AABB combine_with(const AABB& other_box) const;
    AABB multiply_with(const AABB& other_box) const;
    AABB push(const glm::fvec3& vector) const;

public:
    glm::fvec3 min;
    glm::fvec3 max;
};

#endif /* CORE_AABB_HH */
