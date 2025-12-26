#pragma once

class Dimension;

struct Velocity {
    glm::fvec3 value;

public:
    // Updates entities Transform values
    // according to velocities multiplied by fixed_frametime.
    // NOTE: This system was previously called inertial
    static void fixed_update(Dimension* dimension);
};

namespace client
{
// Client-side only - interpolated and previous velocity
struct VelocityIntr final : public Velocity {};
struct VelocityPrev final : public Velocity {};
} // namespace client
