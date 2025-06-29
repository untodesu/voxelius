#ifndef SHARED_VELOCITY_HH
#define SHARED_VELOCITY_HH 1
#pragma once

class Dimension;

struct VelocityComponent final {
    glm::fvec3 value;

public:
    // Updates entities TransformComponent values
    // according to velocities multiplied by fixed_frametime.
    // NOTE: This system was previously called inertial
    static void fixed_update(Dimension* dimension);
};

#endif // SHARED_VELOCITY_HH
