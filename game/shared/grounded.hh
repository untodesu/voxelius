#ifndef SHARED_GROUNDED
#define SHARED_GROUNDED 1
#pragma once

#include "shared/voxel_registry.hh"

// Assigned to entities which are grounded
// according to the collision and gravity system
struct GroundedComponent final {
    voxel_surface surface;
};

#endif /* SHARED_GROUNDED */
