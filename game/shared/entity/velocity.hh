#pragma once

namespace world
{
class Dimension;
} // namespace world

namespace entity
{
struct Velocity final {
    glm::fvec3 value;

public:
    // Updates entities entity::Transform values
    // according to velocities multiplied by fixed_frametime.
    // NOTE: This system was previously called inertial
    static void fixed_update(world::Dimension* dimension);
};
} // namespace entity
