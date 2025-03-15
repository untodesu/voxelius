#ifndef CLIENT_PLAYER_TARGET_HH
#define CLIENT_PLAYER_TARGET_HH 1
#pragma once

#include "shared/voxel_registry.hh"

namespace player_target
{
extern voxel_id voxel;
extern voxel_pos coord;
extern voxel_pos normal;
extern const VoxelInfo *info;
} // namespace player_target

namespace player_target
{
void init(void);
void update(void);
void render(void);
} // namespace player_target

#endif /* CLIENT_PLAYER_TARGET_HH  */
