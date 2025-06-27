#include "shared/pch.hh"

#include "shared/game_voxels.hh"

#include "shared/voxel_registry.hh"

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
    game_voxels::stone = voxel_registry::construct("stone", voxel_type::CUBE, false, false)
                             .add_texture_default("textures/voxel/stone_01.png")
                             .add_texture_default("textures/voxel/stone_02.png")
                             .add_texture_default("textures/voxel/stone_03.png")
                             .add_texture_default("textures/voxel/stone_04.png")
                             .set_surface(voxel_surface::STONE)
                             .build();

    // Cobblestone; should drop when a stone is broken, might also be present in surface features
    game_voxels::cobblestone = voxel_registry::construct("cobblestone", voxel_type::CUBE, false, false)
                                   .add_texture_default("textures/voxel/cobblestone_01.png")
                                   .add_texture_default("textures/voxel/cobblestone_02.png")
                                   .set_surface(voxel_surface::STONE)
                                   .build();

    // Dirt with a grass layer on top; the top layer of plains biome
    game_voxels::grass = voxel_registry::construct("grass", voxel_type::CUBE, false, false)
                             .add_texture_default("textures/voxel/grass_side_01.png")
                             .add_texture_default("textures/voxel/grass_side_02.png")
                             .add_texture(voxel_face::CUBE_BOTTOM, "textures/voxel/dirt_01.png")
                             .add_texture(voxel_face::CUBE_BOTTOM, "textures/voxel/dirt_02.png")
                             .add_texture(voxel_face::CUBE_BOTTOM, "textures/voxel/dirt_03.png")
                             .add_texture(voxel_face::CUBE_BOTTOM, "textures/voxel/dirt_04.png")
                             .add_texture(voxel_face::CUBE_TOP, "textures/voxel/grass_01.png")
                             .add_texture(voxel_face::CUBE_TOP, "textures/voxel/grass_02.png")
                             .set_surface(voxel_surface::GRASS)
                             .build();

    // Dirt; the under-surface layer of some biomes
    game_voxels::dirt = voxel_registry::construct("dirt", voxel_type::CUBE, false, false)
                            .add_texture_default("textures/voxel/dirt_01.png")
                            .add_texture_default("textures/voxel/dirt_02.png")
                            .add_texture_default("textures/voxel/dirt_03.png")
                            .add_texture_default("textures/voxel/dirt_04.png")
                            .set_surface(voxel_surface::DIRT)
                            .build();

    // VTest; a test voxel to ensure animations work
    game_voxels::vtest = voxel_registry::construct("vtest", voxel_type::CUBE, true, false)
                             .add_texture_default("textures/voxel/vtest_F1.png")
                             .add_texture_default("textures/voxel/vtest_F2.png")
                             .add_texture_default("textures/voxel/vtest_F3.png")
                             .add_texture_default("textures/voxel/vtest_F4.png")
                             .build();

    // VTest-CK; a pure blue chromakey I used to make the game's logo
    game_voxels::vtest_ck =
        voxel_registry::construct("vtest_ck", voxel_type::CUBE, false, false).add_texture_default("textures/voxel/chromakey.png").build();

    // Oak leaves; greenery. TODO: add trees as surface features
    game_voxels::oak_leaves = voxel_registry::construct("oak_leaves", voxel_type::CUBE, false, false)
                                  .add_texture_default("textures/voxel/oak_leaves.png")
                                  .set_surface(voxel_surface::GRASS)
                                  .build();

    // Oak planks; the thing that comes out of oak logs
    game_voxels::oak_planks = voxel_registry::construct("oak_planks", voxel_type::CUBE, false, false)
                                  .add_texture_default("textures/voxel/oak_planks_01.png")
                                  .add_texture_default("textures/voxel/oak_planks_02.png")
                                  .set_surface(voxel_surface::WOOD)
                                  .build();

    // Oak logs; greenery. TODO: add trees as surface features
    game_voxels::oak_log = voxel_registry::construct("oak_log", voxel_type::CUBE, false, false)
                               .add_texture_default("textures/voxel/oak_wood_01.png")
                               .add_texture_default("textures/voxel/oak_wood_02.png")
                               .add_texture(voxel_face::CUBE_BOTTOM, "textures/voxel/oak_wood_top.png")
                               .add_texture(voxel_face::CUBE_TOP, "textures/voxel/oak_wood_top.png")
                               .set_surface(voxel_surface::WOOD)
                               .build();

    // Glass; blend rendering test
    game_voxels::glass = voxel_registry::construct("glass", voxel_type::CUBE, false, true)
                             .add_texture_default("textures/voxel/glass_01.png")
                             .set_surface(voxel_surface::GLASS)
                             .build();

    // Slime; it's bouncy!
    game_voxels::slime = voxel_registry::construct("slime", voxel_type::CUBE, false, true)
                             .set_touch(voxel_touch::BOUNCE, glm::fvec3(0.00f, 0.60f, 0.00f))
                             .add_texture_default("textures/voxel/slime_01.png")
                             .build();

    // Mud; you sink in it
    game_voxels::mud = voxel_registry::construct("mud", voxel_type::CUBE, false, false)
                           .set_touch(voxel_touch::SINK, glm::fvec3(0.50f, 0.75f, 0.50f))
                           .add_texture_default("textures/voxel/mud_01.png")
                           .add_texture_default("textures/voxel/mud_02.png")
                           .set_surface(voxel_surface::DIRT)
                           .build();
}
