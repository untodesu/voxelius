#pragma once

namespace world::voxel_anims
{
extern std::uint64_t nextframe;
extern std::uint32_t frame;
} // namespace world::voxel_anims

namespace world::voxel_anims
{
void init(void);
void update(void);
} // namespace world::voxel_anims
