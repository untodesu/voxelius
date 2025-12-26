#pragma once

#include "shared/world/voxel.hh"

// Assigned to entities which are grounded
// according to the collision and gravity system
struct Grounded final {
    VoxelMaterial surface;
};
