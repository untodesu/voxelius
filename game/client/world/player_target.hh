#pragma once

#include "shared/world/voxel_registry.hh"

namespace world::player_target
{
extern voxel_pos coord;
extern voxel_pos normal;
extern const Voxel* voxel;
} // namespace world::player_target

namespace world::player_target
{
void init(void);
void update(void);
void render(void);
} // namespace world::player_target
