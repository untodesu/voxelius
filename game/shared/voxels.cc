// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2024, Voxelius Contributors
#include <game/shared/vdef.hh>
#include <game/shared/voxels.hh>
#include <spdlog/spdlog.h>

Voxel voxels::cobble = NULL_VOXEL;
Voxel voxels::dirt = NULL_VOXEL;
Voxel voxels::grass = NULL_VOXEL;
Voxel voxels::stone = NULL_VOXEL;
Voxel voxels::vtest = NULL_VOXEL;
Voxel voxels::vtest_ck = NULL_VOXEL;
Voxel voxels::oak_leaves = NULL_VOXEL;
Voxel voxels::oak_planks = NULL_VOXEL;
Voxel voxels::oak_wood = NULL_VOXEL;

void voxels::populate(void)
{
    voxels::stone = vdef::create("stone", VoxelType::Cube).add_default_state().build();
    voxels::cobble = vdef::create("cobble", VoxelType::Cube).add_default_state().build();
    voxels::grass = vdef::create("grass", VoxelType::Cube).add_default_state().build();
    voxels::dirt = vdef::create("dirt", VoxelType::Cube).add_default_state().build();
    voxels::vtest = vdef::create("vtest", VoxelType::Cube).add_default_state().add_state("chromakey").build();
    voxels::vtest_ck = voxels::vtest + 1;

    const Voxel leaves_base = vdef::create("leaves", VoxelType::Cube).add_state("oak").build();
    const Voxel planks_base = vdef::create("planks", VoxelType::Cube).add_state("oak").build();
    const Voxel wood_base = vdef::create("wood", VoxelType::Cube).add_state("oak").build();

    voxels::oak_leaves = leaves_base + 0;
    voxels::oak_planks = planks_base + 0;
    voxels::oak_wood = wood_base + 0;
}
