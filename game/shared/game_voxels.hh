#ifndef SHARED_GAME_VOXELS
#define SHARED_GAME_VOXELS 1
#pragma once

#include "shared/types.hh"

namespace game_voxels
{
extern voxel_id cobblestone;
extern voxel_id dirt;
extern voxel_id grass;
extern voxel_id stone;
extern voxel_id vtest;
extern voxel_id vtest_ck;
extern voxel_id oak_leaves;
extern voxel_id oak_planks;
extern voxel_id oak_log;
extern voxel_id glass;
extern voxel_id slime;
extern voxel_id mud;
} // namespace game_voxels

namespace game_voxels
{
void populate(void);
} // namespace game_voxels

#endif // SHARED_GAME_VOXELS
