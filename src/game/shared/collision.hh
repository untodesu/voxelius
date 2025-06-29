#ifndef SHARED_COLLISION_HH
#define SHARED_COLLISION_HH 1
#pragma once

#include "core/aabb.hh"

class Dimension;

struct CollisionComponent final {
    AABB aabb;

public:
    // NOTE: CollisionComponent::fixed_update must be called
    // before TransformComponent::fixed_update and VelocityComponent::fixed_update
    // because both transform and velocity may be updated internally
    static void fixed_update(Dimension* dimension);
};

#endif // SHARED_COLLISION_HH
