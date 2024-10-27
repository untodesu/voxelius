// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2024, Voxelius Contributors
#include <game/shared/game_voxels.hh>
#include <game/shared/vdef.hh>
#include <spdlog/spdlog.h>

Voxel game_voxels::cobble = NULL_VOXEL;
Voxel game_voxels::dirt = NULL_VOXEL;
Voxel game_voxels::grass = NULL_VOXEL;
Voxel game_voxels::stone = NULL_VOXEL;
Voxel game_voxels::vtest = NULL_VOXEL;
Voxel game_voxels::vtest_ck = NULL_VOXEL;
Voxel game_voxels::oak_leaves = NULL_VOXEL;
Voxel game_voxels::oak_planks = NULL_VOXEL;
Voxel game_voxels::oak_wood = NULL_VOXEL;

void game_voxels::populate(void)
{
    game_voxels::stone = vdef::create("stone", VoxelType::Cube).add_default_state().build();
    game_voxels::cobble = vdef::create("cobble", VoxelType::Cube).add_default_state().build();
    game_voxels::grass = vdef::create("grass", VoxelType::Cube).add_default_state().build();
    game_voxels::dirt = vdef::create("dirt", VoxelType::Cube).add_default_state().build();
    game_voxels::vtest = vdef::create("vtest", VoxelType::Cube).add_default_state().add_state("chromakey").build();
    game_voxels::vtest_ck = game_voxels::vtest + 1;

    const Voxel leaves_base = vdef::create("leaves", VoxelType::Cube).add_state("oak").build();
    const Voxel planks_base = vdef::create("planks", VoxelType::Cube).add_state("oak").build();
    const Voxel wood_base = vdef::create("wood", VoxelType::Cube).add_state("oak").build();

    game_voxels::oak_leaves = leaves_base + 0;
    game_voxels::oak_planks = planks_base + 0;
    game_voxels::oak_wood = wood_base + 0;
}
