#include "shared/pch.hh"

#include "shared/game_voxels.hh"

#include "shared/world/dimension.hh"
#include "shared/world/voxel_registry.hh"

#include "shared/const.hh"

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

static void dirt_tick(world::Dimension* dimension, const voxel_pos& vpos)
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

    if(above_voxel == nullptr || above_voxel->is_surface_material<world::VMAT_GLASS>()) {
        air_above = true;
    }

    if(grass_found && air_above) {
        // Replace itself with the grass voxel
        dimension->set_voxel(game_voxels::grass, vpos);
    }
}

void game_voxels::populate(void)
{
    world::VoxelBuilder builder;

    builder = world::VoxelBuilder("stone");
    builder.add_default_texture("textures/voxel/stone_01.png")
        .add_default_texture("textures/voxel/stone_02.png")
        .add_default_texture("textures/voxel/stone_03.png")
        .add_default_texture("textures/voxel/stone_04.png");
    stone = world::voxel_registry::register_voxel(builder);

    builder = world::VoxelBuilder("cobblestone");
    builder.add_default_texture("textures/voxel/cobblestone_01.png").add_default_texture("textures/voxel/cobblestone_02.png");
    cobblestone = world::voxel_registry::register_voxel(builder);

    builder = world::VoxelBuilder("dirt");
    builder.add_default_texture("textures/voxel/dirt_01.png")
        .add_default_texture("textures/voxel/dirt_02.png")
        .add_default_texture("textures/voxel/dirt_03.png")
        .add_default_texture("textures/voxel/dirt_04.png");
    builder.set_surface_material(world::VMAT_DIRT);
    builder.set_on_tick(&dirt_tick);
    dirt = world::voxel_registry::register_voxel(builder);

    builder = world::VoxelBuilder("grass");
    builder.add_default_texture("textures/voxel/grass_side_01.png")
        .add_default_texture("textures/voxel/grass_side_02.png")
        .add_face_texture(world::VFACE_BOTTOM, "textures/voxel/dirt_01.png")
        .add_face_texture(world::VFACE_BOTTOM, "textures/voxel/dirt_02.png")
        .add_face_texture(world::VFACE_BOTTOM, "textures/voxel/dirt_03.png")
        .add_face_texture(world::VFACE_BOTTOM, "textures/voxel/dirt_04.png")
        .add_face_texture(world::VFACE_TOP, "textures/voxel/grass_01.png")
        .add_face_texture(world::VFACE_TOP, "textures/voxel/grass_02.png");
    builder.set_surface_material(world::VMAT_GRASS);
    grass = world::voxel_registry::register_voxel(builder);

    builder = world::VoxelBuilder("vtest");
    builder.add_default_texture("textures/voxel/vtest_F1.png")
        .add_default_texture("textures/voxel/vtest_F2.png")
        .add_default_texture("textures/voxel/vtest_F3.png")
        .add_default_texture("textures/voxel/vtest_F4.png");
    builder.set_animated(true);
    vtest = world::voxel_registry::register_voxel(builder);

    builder = world::VoxelBuilder("vtest_ck");
    builder.add_default_texture("textures/voxel/chromakey.png");
    vtest_ck = world::voxel_registry::register_voxel(builder);

    builder = world::VoxelBuilder("oak_leaves");
    builder.add_default_texture("textures/voxel/oak_leaves.png");
    builder.set_render_mode(world::VRENDER_BLEND);
    builder.set_surface_material(world::VMAT_GRASS);
    oak_leaves = world::voxel_registry::register_voxel(builder);

    builder = world::VoxelBuilder("oak_planks");
    builder.add_default_texture("textures/voxel/oak_planks_01.png").add_default_texture("textures/voxel/oak_planks_02.png");
    builder.set_surface_material(world::VMAT_WOOD);
    oak_planks = world::voxel_registry::register_voxel(builder);

    builder = world::VoxelBuilder("oak_log");
    builder.add_default_texture("textures/voxel/oak_wood_01.png")
        .add_default_texture("textures/voxel/oak_wood_02.png")
        .add_face_texture(world::VFACE_BOTTOM, "textures/voxel/oak_wood_top.png")
        .add_face_texture(world::VFACE_TOP, "textures/voxel/oak_wood_top.png");
    builder.set_surface_material(world::VMAT_WOOD);
    oak_log = world::voxel_registry::register_voxel(builder);

    builder = world::VoxelBuilder("glass");
    builder.add_default_texture("textures/voxel/glass_01.png");
    builder.set_render_mode(world::VRENDER_BLEND);
    builder.set_surface_material(world::VMAT_GLASS);
    glass = world::voxel_registry::register_voxel(builder);

    builder = world::VoxelBuilder("slime");
    builder.add_default_texture("textures/voxel/slime_01.png");
    builder.set_render_mode(world::VRENDER_BLEND);
    builder.set_surface_material(world::VMAT_SLOSH);
    builder.set_touch_type(world::VTOUCH_BOUNCE).set_touch_values({ 0.00f, 0.60f, 0.00f });
    slime = world::voxel_registry::register_voxel(builder);
}
