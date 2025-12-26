// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: game_voxels.cc
// Description: All the voxels in the game

#include "shared/pch.hh"

#include "shared/game_voxels.hh"

#include "shared/world/dimension.hh"
#include "shared/world/voxel_registry.hh"

#include "shared/const.hh"

const Voxel* game_voxels::cobblestone = nullptr;
const Voxel* game_voxels::dirt = nullptr;
const Voxel* game_voxels::grass = nullptr;
const Voxel* game_voxels::stone = nullptr;
const Voxel* game_voxels::vtest = nullptr;
const Voxel* game_voxels::vtest_ck = nullptr;
const Voxel* game_voxels::oak_leaves = nullptr;
const Voxel* game_voxels::oak_planks = nullptr;
const Voxel* game_voxels::oak_log = nullptr;
const Voxel* game_voxels::glass = nullptr;
const Voxel* game_voxels::slime = nullptr;

static void dirt_tick(Dimension* dimension, const voxel_pos& vpos)
{
    auto grass_found = false;
    auto air_above = false;

    for(voxel_pos::value_type dx = -1; dx <= 1 && !grass_found; ++dx) {
        for(voxel_pos::value_type dy = -1; dy <= 1 && !grass_found; ++dy) {
            for(voxel_pos::value_type dz = -1; dz <= 1 && !grass_found; ++dz) {
                if(dx == 0 && dy == 0 && dz == 0) {
                    // Skip self
                    continue;
                }

                auto neighbour_vpos = vpos + voxel_pos(dx, dy, dz);
                auto neighbour_voxel = dimension->get_voxel(neighbour_vpos);

                // Voxel pointers returned by get_voxel() are the exact same
                // returned by the voxel registry, so we can compare pointers directly
                // and not bother with voxel_id property comparisons
                if(neighbour_voxel == game_voxels::grass) {
                    grass_found = true;
                    break;
                }
            }
        }
    }

    auto above_vpos = vpos + voxel_pos(0, 1, 0);
    auto above_voxel = dimension->get_voxel(above_vpos);

    if(above_voxel == nullptr || above_voxel->is_surface_material<VMAT_GLASS>()) {
        air_above = true;
    }

    if(grass_found && air_above) {
        dimension->set_voxel(game_voxels::grass, vpos);
    }
}

static void grass_tick(Dimension* dimension, const voxel_pos& vpos)
{
    auto above_vpos = vpos + voxel_pos(0, 1, 0);
    auto above_voxel = dimension->get_voxel(above_vpos);

    if(above_voxel && !above_voxel->is_surface_material<VMAT_GLASS>()) {
        // Decay into dirt if something is blocking airflow
        dimension->set_voxel(game_voxels::dirt, vpos);
    }
}

void game_voxels::populate(void)
{
    auto stone_builder = VoxelBuilder("stone");
    stone_builder.add_default_texture("textures/voxel/stone_01.png");
    stone_builder.add_default_texture("textures/voxel/stone_02.png");
    stone_builder.add_default_texture("textures/voxel/stone_03.png");
    stone_builder.add_default_texture("textures/voxel/stone_04.png");
    stone = voxel_registry::register_voxel(stone_builder);

    auto cobblestone_builder = VoxelBuilder("cobblestone");
    cobblestone_builder.add_default_texture("textures/voxel/cobblestone_01.png");
    cobblestone_builder.add_default_texture("textures/voxel/cobblestone_02.png");
    cobblestone = voxel_registry::register_voxel(cobblestone_builder);

    auto dirt_builder = VoxelBuilder("dirt");
    dirt_builder.add_default_texture("textures/voxel/dirt_01.png");
    dirt_builder.add_default_texture("textures/voxel/dirt_02.png");
    dirt_builder.add_default_texture("textures/voxel/dirt_03.png");
    dirt_builder.add_default_texture("textures/voxel/dirt_04.png");
    dirt_builder.set_surface_material(VMAT_DIRT);
    dirt_builder.set_on_tick(&dirt_tick);
    dirt = voxel_registry::register_voxel(dirt_builder);

    auto grass_builder = VoxelBuilder("grass");
    grass_builder.add_default_texture("textures/voxel/grass_side_01.png");
    grass_builder.add_default_texture("textures/voxel/grass_side_02.png");
    grass_builder.add_face_texture(VFACE_BOTTOM, "textures/voxel/dirt_01.png");
    grass_builder.add_face_texture(VFACE_BOTTOM, "textures/voxel/dirt_02.png");
    grass_builder.add_face_texture(VFACE_BOTTOM, "textures/voxel/dirt_03.png");
    grass_builder.add_face_texture(VFACE_BOTTOM, "textures/voxel/dirt_04.png");
    grass_builder.add_face_texture(VFACE_TOP, "textures/voxel/grass_01.png");
    grass_builder.add_face_texture(VFACE_TOP, "textures/voxel/grass_02.png");
    grass_builder.set_surface_material(VMAT_GRASS);
    grass_builder.set_on_tick(&grass_tick);
    grass = voxel_registry::register_voxel(grass_builder);

    auto vtest_builder = VoxelBuilder("vtest");
    vtest_builder.add_default_texture("textures/voxel/vtest_F1.png");
    vtest_builder.add_default_texture("textures/voxel/vtest_F2.png");
    vtest_builder.add_default_texture("textures/voxel/vtest_F3.png");
    vtest_builder.add_default_texture("textures/voxel/vtest_F4.png");
    vtest_builder.set_animated(true);
    vtest = voxel_registry::register_voxel(vtest_builder);

    auto vtest_ck_builder = VoxelBuilder("vtest_ck");
    vtest_ck_builder.add_default_texture("textures/voxel/chromakey.png");
    vtest_ck = voxel_registry::register_voxel(vtest_ck_builder);

    auto oak_leaves_builder = VoxelBuilder("oak_leaves");
    oak_leaves_builder.add_default_texture("textures/voxel/oak_leaves.png");
    oak_leaves_builder.set_surface_material(VMAT_GRASS);
    oak_leaves = voxel_registry::register_voxel(oak_leaves_builder);

    auto oak_planks_builder = VoxelBuilder("oak_planks");
    oak_planks_builder.add_default_texture("textures/voxel/oak_planks_01.png");
    oak_planks_builder.add_default_texture("textures/voxel/oak_planks_02.png");
    oak_planks_builder.set_surface_material(VMAT_WOOD);
    oak_planks = voxel_registry::register_voxel(oak_planks_builder);

    auto oak_log_builder = VoxelBuilder("oak_log");
    oak_log_builder.add_default_texture("textures/voxel/oak_wood_01.png");
    oak_log_builder.add_default_texture("textures/voxel/oak_wood_02.png");
    oak_log_builder.add_face_texture(VFACE_BOTTOM, "textures/voxel/oak_wood_top.png");
    oak_log_builder.add_face_texture(VFACE_TOP, "textures/voxel/oak_wood_top.png");
    oak_log_builder.set_surface_material(VMAT_WOOD);
    oak_log = voxel_registry::register_voxel(oak_log_builder);

    auto glass_builder = VoxelBuilder("glass");
    glass_builder.add_default_texture("textures/voxel/glass_01.png");
    glass_builder.set_render_mode(VRENDER_BLEND);
    glass_builder.set_surface_material(VMAT_GLASS);
    glass = voxel_registry::register_voxel(glass_builder);

    auto slime_builder = VoxelBuilder("slime");
    slime_builder.add_default_texture("textures/voxel/slime_01.png");
    slime_builder.set_render_mode(VRENDER_BLEND);
    slime_builder.set_surface_material(VMAT_SLOSH);
    slime_builder.set_touch_type(VTOUCH_BOUNCE);
    slime_builder.set_touch_values({ 0.00f, 0.60f, 0.00f });
    slime = voxel_registry::register_voxel(slime_builder);
}
