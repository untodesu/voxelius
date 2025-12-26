// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: game_voxels.hh
// Description: All the voxels in the game

#ifndef SHARED_GAME_VOXELS_HH
#define SHARED_GAME_VOXELS_HH
#pragma once

class Voxel;

namespace game_voxels
{
extern const Voxel* cobblestone;
extern const Voxel* dirt;
extern const Voxel* grass;
extern const Voxel* stone;
extern const Voxel* vtest;
extern const Voxel* vtest_ck;
extern const Voxel* oak_leaves;
extern const Voxel* oak_planks;
extern const Voxel* oak_log;
extern const Voxel* glass;
extern const Voxel* slime;
} // namespace game_voxels

namespace game_voxels
{
void populate(void);
} // namespace game_voxels

#endif
