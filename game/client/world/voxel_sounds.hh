#pragma once

#include "core/resource/resource.hh"

#include "shared/world/voxel_registry.hh"

struct SoundEffect;

namespace world::voxel_sounds
{
void init(void);
void shutdown(void);
} // namespace world::voxel_sounds

namespace world::voxel_sounds
{
resource_ptr<SoundEffect> get_footsteps(voxel_surface surface);
resource_ptr<SoundEffect> get_placebreak(voxel_surface surface);
} // namespace world::voxel_sounds
