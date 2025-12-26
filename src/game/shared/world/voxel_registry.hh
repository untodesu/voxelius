#pragma once

#include "shared/world/voxel.hh"

namespace voxel_registry
{
extern emhash8::HashMap<std::string, voxel_id> names;
extern std::vector<std::unique_ptr<Voxel>> voxels;
} // namespace voxel_registry

namespace voxel_registry
{
Voxel* register_voxel(const VoxelBuilder& builder);
Voxel* find(std::string_view name);
Voxel* find(voxel_id id);
} // namespace voxel_registry

namespace voxel_registry
{
void purge(void);
} // namespace voxel_registry

namespace voxel_registry
{
std::uint64_t get_checksum(void);
} // namespace voxel_registry
