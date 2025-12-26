// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: player_target.hh
// Description: Handle logic for targeting voxels

#ifndef CLIENT_WORLD_PLAYER_TARGET_HH
#define CLIENT_WORLD_PLAYER_TARGET_HH
#pragma once

#include "shared/world/voxel_registry.hh"

namespace player_target
{
extern voxel_pos coord;
extern voxel_pos normal;
extern const Voxel* voxel;
} // namespace player_target

namespace player_target
{
void init(void);
void update(void);
void render(void);
} // namespace player_target

#endif
