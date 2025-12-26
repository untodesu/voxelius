// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: voxel_sounds.hh
// Description: Handle voxel sounds

#ifndef CLIENT_WORLD_VOXEL_SOUNDS_HH
#define CLIENT_WORLD_VOXEL_SOUNDS_HH
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

#endif
