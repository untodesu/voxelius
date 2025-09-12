#pragma once

namespace world
{
class Voxel;
} // namespace world

namespace game_voxels
{
extern const world::Voxel* cobblestone;
extern const world::Voxel* dirt;
extern const world::Voxel* grass;
extern const world::Voxel* stone;
extern const world::Voxel* vtest;
extern const world::Voxel* vtest_ck;
extern const world::Voxel* oak_leaves;
extern const world::Voxel* oak_planks;
extern const world::Voxel* oak_log;
extern const world::Voxel* glass;
extern const world::Voxel* slime;
} // namespace game_voxels

namespace game_voxels
{
void populate(void);
} // namespace game_voxels
