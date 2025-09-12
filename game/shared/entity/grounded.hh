#pragma once

#include "shared/world/voxel.hh"

namespace entity
{
// Assigned to entities which are grounded
// according to the collision and gravity system
struct Grounded final {
    world::VoxelMaterial surface;
};
} // namespace entity
