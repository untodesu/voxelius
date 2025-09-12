#ifndef SHARED_ENTITY_GROUNDED
#define SHARED_ENTITY_GROUNDED 1
#pragma once

#include "shared/world/voxel_registry.hh"

namespace entity
{
// Assigned to entities which are grounded
// according to the collision and gravity system
struct Grounded final {
    world::VoxelMaterial surface;
};
} // namespace entity

#endif // SHARED_ENTITY_GROUNDED
