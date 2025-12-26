// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: game_items.cc
// Description: All the items in the game

#include "shared/pch.hh"

#include "shared/game_items.hh"

#include "shared/world/item_registry.hh"

#include "shared/game_voxels.hh"

const Item* game_items::stone = nullptr;
const Item* game_items::cobblestone = nullptr;
const Item* game_items::dirt = nullptr;
const Item* game_items::grass = nullptr;
const Item* game_items::oak_leaves = nullptr;
const Item* game_items::oak_planks = nullptr;
const Item* game_items::oak_log = nullptr;
const Item* game_items::glass = nullptr;
const Item* game_items::slime = nullptr;

void game_items::populate(void)
{
    auto stone_builder = ItemBuilder("stone");
    stone_builder.set_texture("textures/item/stone.png");
    stone_builder.set_place_voxel(game_voxels::stone);
    stone = item_registry::register_item(stone_builder);

    auto cobblestone_builder = ItemBuilder("cobblestone");
    cobblestone_builder.set_texture("textures/item/cobblestone.png");
    cobblestone_builder.set_place_voxel(game_voxels::cobblestone);
    cobblestone = item_registry::register_item(cobblestone_builder);

    auto dirt_builder = ItemBuilder("dirt");
    dirt_builder.set_texture("textures/item/dirt.png");
    dirt_builder.set_place_voxel(game_voxels::dirt);
    dirt = item_registry::register_item(dirt_builder);

    auto grass_builder = ItemBuilder("grass");
    grass_builder.set_texture("textures/item/grass.png");
    grass_builder.set_place_voxel(game_voxels::grass);
    grass = item_registry::register_item(grass_builder);

    auto oak_leaves_builder = ItemBuilder("oak_leaves");
    oak_leaves_builder.set_texture("textures/item/oak_leaves.png");
    oak_leaves_builder.set_place_voxel(game_voxels::oak_leaves);
    oak_leaves = item_registry::register_item(oak_leaves_builder);

    auto oak_planks_builder = ItemBuilder("oak_planks");
    oak_planks_builder.set_texture("textures/item/oak_planks.png");
    oak_planks_builder.set_place_voxel(game_voxels::oak_planks);
    oak_planks = item_registry::register_item(oak_planks_builder);

    auto oak_log_builder = ItemBuilder("oak_log");
    oak_log_builder.set_texture("textures/item/oak_log.png");
    oak_log_builder.set_place_voxel(game_voxels::oak_log);
    oak_log = item_registry::register_item(oak_log_builder);

    auto glass_builder = ItemBuilder("glass");
    glass_builder.set_texture("textures/item/glass.png");
    glass_builder.set_place_voxel(game_voxels::glass);
    glass = item_registry::register_item(glass_builder);

    auto slime_builder = ItemBuilder("slime");
    slime_builder.set_texture("textures/item/slime.png");
    slime_builder.set_place_voxel(game_voxels::slime);
    slime = item_registry::register_item(slime_builder);
}
