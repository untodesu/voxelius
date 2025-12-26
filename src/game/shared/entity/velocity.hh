#pragma once

class Dimension;

struct Velocity final {
    glm::fvec3 value;

public:
    // Updates entities Transform values
    // according to velocities multiplied by fixed_frametime.
    // NOTE: This system was previously called inertial
    static void fixed_update(Dimension* dimension);
};
