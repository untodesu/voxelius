#pragma once

#include "core/math/aabb.hh"

class Dimension;

struct Collision final {
    math::AABBf aabb;

public:
    // NOTE: Collision::fixed_update must be called
    // before Transform::fixed_update and Velocity::fixed_update
    // because both transform and velocity may be updated internally
    static void fixed_update(Dimension* dimension);
};
