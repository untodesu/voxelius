#include "shared/pch.hh"

#include "shared/game_voxels.hh"

#include "shared/world/voxel_registry.hh"

voxel_id game_voxels::cobblestone = NULL_VOXEL_ID;
voxel_id game_voxels::dirt = NULL_VOXEL_ID;
voxel_id game_voxels::grass = NULL_VOXEL_ID;
voxel_id game_voxels::stone = NULL_VOXEL_ID;
voxel_id game_voxels::vtest = NULL_VOXEL_ID;
voxel_id game_voxels::vtest_ck = NULL_VOXEL_ID;
voxel_id game_voxels::oak_leaves = NULL_VOXEL_ID;
voxel_id game_voxels::oak_planks = NULL_VOXEL_ID;
voxel_id game_voxels::oak_log = NULL_VOXEL_ID;
voxel_id game_voxels::glass = NULL_VOXEL_ID;
voxel_id game_voxels::slime = NULL_VOXEL_ID;
voxel_id game_voxels::mud = NULL_VOXEL_ID;

void game_voxels::populate(void)
{
    // Stone; the backbone of the generated world
    game_voxels::stone =
        world::voxel_registry::construct("stone", world::voxel_type::CUBE, false, false)
            .add_texture_default("textures/voxel/stone_01.png")
            .add_texture_default("textures/voxel/stone_02.png")
            .add_texture_default("textures/voxel/stone_03.png")
            .add_texture_default("textures/voxel/stone_04.png")
            .set_surface(world::voxel_surface::STONE)
            .build();

    // Cobblestone; should drop when a stone is broken, might also be present in surface features
    game_voxels::cobblestone =
        world::voxel_registry::construct("cobblestone", world::voxel_type::CUBE, false, false)
            .add_texture_default("textures/voxel/cobblestone_01.png")
            .add_texture_default("textures/voxel/cobblestone_02.png")
            .set_surface(world::voxel_surface::STONE)
            .build();

    // Dirt with a grass layer on top; the top layer of plains biome
    game_voxels::grass =
        world::voxel_registry::construct("grass", world::voxel_type::CUBE, false, false)
            .add_texture_default("textures/voxel/grass_side_01.png")
            .add_texture_default("textures/voxel/grass_side_02.png")
            .add_texture(world::voxel_face::CUBE_BOTTOM, "textures/voxel/dirt_01.png")
            .add_texture(world::voxel_face::CUBE_BOTTOM, "textures/voxel/dirt_02.png")
            .add_texture(world::voxel_face::CUBE_BOTTOM, "textures/voxel/dirt_03.png")
            .add_texture(world::voxel_face::CUBE_BOTTOM, "textures/voxel/dirt_04.png")
            .add_texture(world::voxel_face::CUBE_TOP, "textures/voxel/grass_01.png")
            .add_texture(world::voxel_face::CUBE_TOP, "textures/voxel/grass_02.png")
            .set_surface(world::voxel_surface::GRASS)
            .build();

    // Dirt; the under-surface layer of some biomes
    game_voxels::dirt =
        world::voxel_registry::construct("dirt", world::voxel_type::CUBE, false, false)
            .add_texture_default("textures/voxel/dirt_01.png")
            .add_texture_default("textures/voxel/dirt_02.png")
            .add_texture_default("textures/voxel/dirt_03.png")
            .add_texture_default("textures/voxel/dirt_04.png")
            .set_surface(world::voxel_surface::DIRT)
            .build();

    // VTest; a test voxel to ensure animations work
    game_voxels::vtest = world::voxel_registry::construct("vtest", world::voxel_type::CUBE, true, false)
                             .add_texture_default("textures/voxel/vtest_F1.png")
                             .add_texture_default("textures/voxel/vtest_F2.png")
                             .add_texture_default("textures/voxel/vtest_F3.png")
                             .add_texture_default("textures/voxel/vtest_F4.png")
                             .build();

    // VTest-CK; a pure blue chromakey I used to make the game's logo
    game_voxels::vtest_ck = world::voxel_registry::construct("vtest_ck", world::voxel_type::CUBE, false, false)
                                .add_texture_default("textures/voxel/chromakey.png")
                                .build();

    // Oak leaves; greenery. TODO: add trees as surface features
    game_voxels::oak_leaves = world::voxel_registry::construct("oak_leaves", world::voxel_type::CUBE, false, false)
                                  .add_texture_default("textures/voxel/oak_leaves.png")
                                  .set_surface(world::voxel_surface::GRASS)
                                  .build();

    // Oak planks; the thing that comes out of oak logs
    game_voxels::oak_planks = world::voxel_registry::construct("oak_planks", world::voxel_type::CUBE, false, false)
                                  .add_texture_default("textures/voxel/oak_planks_01.png")
                                  .add_texture_default("textures/voxel/oak_planks_02.png")
                                  .set_surface(world::voxel_surface::WOOD)
                                  .build();

    // Oak logs; greenery. TODO: add trees as surface features
    game_voxels::oak_log =
        world::voxel_registry::construct("oak_log", world::voxel_type::CUBE, false, false)
            .add_texture_default("textures/voxel/oak_wood_01.png")
            .add_texture_default("textures/voxel/oak_wood_02.png")
            .add_texture(world::voxel_face::CUBE_BOTTOM, "textures/voxel/oak_wood_top.png")
            .add_texture(world::voxel_face::CUBE_TOP, "textures/voxel/oak_wood_top.png")
            .set_surface(world::voxel_surface::WOOD)
            .build();

    // Glass; blend rendering test
    game_voxels::glass = world::voxel_registry::construct("glass", world::voxel_type::CUBE, false, true)
                             .add_texture_default("textures/voxel/glass_01.png")
                             .set_surface(world::voxel_surface::GLASS)
                             .build();

    // Slime; it's bouncy!
    game_voxels::slime = world::voxel_registry::construct("slime", world::voxel_type::CUBE, false, true)
                             .set_touch(world::voxel_touch::BOUNCE, glm::fvec3(0.00f, 0.60f, 0.00f))
                             .add_texture_default("textures/voxel/slime_01.png")
                             .build();

    // Mud; you sink in it
    game_voxels::mud = world::voxel_registry::construct("mud", world::voxel_type::CUBE, false, false)
                           .set_touch(world::voxel_touch::SINK, glm::fvec3(0.50f, 0.75f, 0.50f))
                           .add_texture_default("textures/voxel/mud_01.png")
                           .add_texture_default("textures/voxel/mud_02.png")
                           .set_surface(world::voxel_surface::DIRT)
                           .build();
}
