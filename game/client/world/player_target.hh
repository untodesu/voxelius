#ifndef CLIENT_PLAYER_TARGET_HH
#define CLIENT_PLAYER_TARGET_HH 1
#pragma once

#include "shared/world/voxel_registry.hh"

namespace world::player_target
{
extern voxel_id voxel;
extern voxel_pos coord;
extern voxel_pos normal;
extern const VoxelInfo* info;
} // namespace world::player_target

namespace world::player_target
{
void init(void);
void update(void);
void render(void);
} // namespace world::player_target

#endif // CLIENT_PLAYER_TARGET_HH
