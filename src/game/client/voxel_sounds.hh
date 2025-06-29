#ifndef CLIENT_VOXEL_SOUNDS_HH
#define CLIENT_VOXEL_SOUNDS_HH 1
#pragma once

#include "core/resource.hh"

#include "shared/voxel_registry.hh"

struct SoundEffect;

namespace voxel_sounds
{
void init(void);
void shutdown(void);
} // namespace voxel_sounds

namespace voxel_sounds
{
resource_ptr<SoundEffect> get_footsteps(voxel_surface surface);
resource_ptr<SoundEffect> get_placebreak(voxel_surface surface);
} // namespace voxel_sounds

#endif // CLIENT_VOXEL_SOUNDS_HH
