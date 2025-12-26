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
