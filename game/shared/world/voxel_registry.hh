#pragma once

#include "shared/world/voxel.hh"

namespace world::voxel_registry
{
extern emhash8::HashMap<std::string, voxel_id> names;
extern std::vector<std::shared_ptr<Voxel>> voxels;
} // namespace world::voxel_registry

namespace world::voxel_registry
{
Voxel* register_voxel(const Voxel& voxel_template);
Voxel* find(std::string_view name);
Voxel* find(voxel_id id);
} // namespace world::voxel_registry

namespace world::voxel_registry
{
void purge(void);
} // namespace world::voxel_registry

namespace world::voxel_registry
{
std::uint64_t get_checksum(void);
} // namespace world::voxel_registry
