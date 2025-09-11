#pragma once

#include "core/math/aabb.hh"

namespace world
{
class Dimension;
} // namespace world

namespace entity
{
struct Collision final {
    math::AABB aabb;

public:
    // NOTE: entity::Collision::fixed_update must be called
    // before entity::Transform::fixed_update and entity::Velocity::fixed_update
    // because both transform and velocity may be updated internally
    static void fixed_update(world::Dimension* dimension);
};
} // namespace entity
