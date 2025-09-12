#include "shared/pch.hh"

#include "shared/game_items.hh"

#include "shared/world/item_registry.hh"

#include "shared/game_voxels.hh"

const world::Item* game_items::stone = nullptr;
const world::Item* game_items::cobblestone = nullptr;
const world::Item* game_items::dirt = nullptr;
const world::Item* game_items::grass = nullptr;
const world::Item* game_items::oak_leaves = nullptr;
const world::Item* game_items::oak_planks = nullptr;
const world::Item* game_items::oak_log = nullptr;
const world::Item* game_items::glass = nullptr;
const world::Item* game_items::slime = nullptr;

void game_items::populate(void)
{
    auto stone_builder = world::ItemBuilder("stone");
    stone_builder.set_texture("textures/item/stone.png");
    stone_builder.set_place_voxel(game_voxels::stone);
    stone = world::item_registry::register_item(stone_builder);

    auto cobblestone_builder = world::ItemBuilder("cobblestone");
    cobblestone_builder.set_texture("textures/item/cobblestone.png");
    cobblestone_builder.set_place_voxel(game_voxels::cobblestone);
    cobblestone = world::item_registry::register_item(cobblestone_builder);

    auto dirt_builder = world::ItemBuilder("dirt");
    dirt_builder.set_texture("textures/item/dirt.png");
    dirt_builder.set_place_voxel(game_voxels::dirt);
    dirt = world::item_registry::register_item(dirt_builder);

    auto grass_builder = world::ItemBuilder("grass");
    grass_builder.set_texture("textures/item/grass.png");
    grass_builder.set_place_voxel(game_voxels::grass);
    grass = world::item_registry::register_item(grass_builder);

    auto oak_leaves_builder = world::ItemBuilder("oak_leaves");
    oak_leaves_builder.set_texture("textures/item/oak_leaves.png");
    oak_leaves_builder.set_place_voxel(game_voxels::oak_leaves);
    oak_leaves = world::item_registry::register_item(oak_leaves_builder);

    auto oak_planks_builder = world::ItemBuilder("oak_planks");
    oak_planks_builder.set_texture("textures/item/oak_planks.png");
    oak_planks_builder.set_place_voxel(game_voxels::oak_planks);
    oak_planks = world::item_registry::register_item(oak_planks_builder);

    auto oak_log_builder = world::ItemBuilder("oak_log");
    oak_log_builder.set_texture("textures/item/oak_log.png");
    oak_log_builder.set_place_voxel(game_voxels::oak_log);
    oak_log = world::item_registry::register_item(oak_log_builder);

    auto glass_builder = world::ItemBuilder("glass");
    glass_builder.set_texture("textures/item/glass.png");
    glass_builder.set_place_voxel(game_voxels::glass);
    glass = world::item_registry::register_item(glass_builder);

    auto slime_builder = world::ItemBuilder("slime");
    slime_builder.set_texture("textures/item/slime.png");
    slime_builder.set_place_voxel(game_voxels::slime);
    slime = world::item_registry::register_item(slime_builder);
}
