// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: voxel_registry.hh
// Description: Registry for all the voxels in the game

#ifndef SHARED_WORLD_VOXEL_REGISTRY_HH
#define SHARED_WORLD_VOXEL_REGISTRY_HH
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

#endif
