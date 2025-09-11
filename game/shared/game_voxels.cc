#include "shared/pch.hh"

#include "shared/game_voxels.hh"

#include "shared/world/voxels/generic.hh"

#include "shared/world/voxel_registry.hh"

const world::Voxel* game_voxels::cobblestone = nullptr;
const world::Voxel* game_voxels::dirt = nullptr;
const world::Voxel* game_voxels::grass = nullptr;
const world::Voxel* game_voxels::stone = nullptr;
const world::Voxel* game_voxels::vtest = nullptr;
const world::Voxel* game_voxels::vtest_ck = nullptr;
const world::Voxel* game_voxels::oak_leaves = nullptr;
const world::Voxel* game_voxels::oak_planks = nullptr;
const world::Voxel* game_voxels::oak_log = nullptr;
const world::Voxel* game_voxels::glass = nullptr;
const world::Voxel* game_voxels::slime = nullptr;
const world::Voxel* game_voxels::mud = nullptr;

void game_voxels::populate(void)
{
    using namespace world;
    using namespace world::voxels;
    using world::voxel_registry::register_voxel;

    stone = register_voxel(GenericCube("stone", VRENDER_OPAQUE, false, VMAT_STONE, VTOUCH_SOLID, glm::fvec3(0.0f, 0.0f, 0.0f),
        "textures/voxel/stone_01.png", "textures/voxel/stone_02.png", "textures/voxel/stone_03.png", "textures/voxel/stone_04.png"));

    cobblestone = register_voxel(GenericCube("cobblestone", VRENDER_OPAQUE, false, VMAT_STONE, VTOUCH_SOLID, glm::fvec3(0.0f, 0.0f, 0.0f),
        "textures/voxel/cobblestone_01.png", "textures/voxel/cobblestone_02.png"));

    vtest = register_voxel(GenericCube("vtest", VRENDER_OPAQUE, true, VMAT_DEFAULT, VTOUCH_SOLID, glm::fvec3(0.0f, 0.0f, 0.0f),
        "textures/voxel/vtest_F1.png", "textures/voxel/vtest_F2.png", "textures/voxel/vtest_F3.png", "textures/voxel/vtest_F4.png"));

    vtest_ck = register_voxel(GenericCube("vtest_ck", VRENDER_OPAQUE, false, VMAT_DEFAULT, VTOUCH_SOLID, glm::fvec3(0.0f, 0.0f, 0.0f),
        "textures/voxel/chromakey.png"));

    oak_leaves = register_voxel(GenericCube("oak_leaves", VRENDER_BLEND, false, VMAT_GRASS, VTOUCH_SOLID, glm::fvec3(0.0f, 0.0f, 0.0f),
        "textures/voxel/oak_leaves.png"));

    oak_planks = register_voxel(GenericCube("oak_planks", VRENDER_OPAQUE, false, VMAT_WOOD, VTOUCH_SOLID, glm::fvec3(0.0f, 0.0f, 0.0f),
        "textures/voxel/oak_planks_01.png", "textures/voxel/oak_planks_02.png"));

    glass = register_voxel(GenericCube("glass", VRENDER_BLEND, false, VMAT_GLASS, VTOUCH_SOLID, glm::fvec3(0.0f, 0.0f, 0.0f),
        "textures/voxel/glass_01.png"));

    slime = register_voxel(GenericCube("slime", VRENDER_BLEND, false, VMAT_SLOSH, VTOUCH_BOUNCE, glm::fvec3(0.00f, 0.60f, 0.00f),
        "textures/voxel/slime_01.png"));

#if 0

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
#endif
}
