#pragma once

#include "core/resource/resource.hh"

#include "shared/world/voxel.hh"

struct SoundEffect;

namespace voxel_sounds
{
void init(void);
void shutdown(void);
} // namespace voxel_sounds

namespace voxel_sounds
{
resource_ptr<SoundEffect> get_footsteps(VoxelMaterial material);
resource_ptr<SoundEffect> get_placebreak(VoxelMaterial material);
} // namespace voxel_sounds
