#include "client/pch.hh"

#include "client/world/player_target.hh"

#include "shared/world/dimension.hh"
#include "shared/world/ray_dda.hh"

#include "shared/coord.hh"

#include "client/entity/camera.hh"
#include "client/world/outline.hh"

#include "client/game.hh"
#include "client/globals.hh"
#include "client/session.hh"

constexpr static float MAX_REACH = 16.0f;

voxel_id world::player_target::voxel;
voxel_pos world::player_target::coord;
voxel_pos world::player_target::normal;
const world::VoxelInfo* world::player_target::info;

void world::player_target::init(void)
{
    world::player_target::voxel = NULL_VOXEL_ID;
    world::player_target::coord = voxel_pos();
    world::player_target::normal = voxel_pos();
    world::player_target::info = nullptr;
}

void world::player_target::update(void)
{
    if(session::is_ingame()) {
        RayDDA ray(globals::dimension, entity::camera::position_chunk, entity::camera::position_local, entity::camera::direction);

        do {
            world::player_target::voxel = ray.step();

            if(world::player_target::voxel != NULL_VOXEL_ID) {
                world::player_target::coord = ray.vpos;
                world::player_target::normal = ray.vnormal;
                world::player_target::info = world::voxel_registry::find(world::player_target::voxel);
                break;
            }

            world::player_target::coord = voxel_pos();
            world::player_target::normal = voxel_pos();
            world::player_target::info = nullptr;
        } while(ray.distance < MAX_REACH);
    }
    else {
        world::player_target::voxel = NULL_VOXEL_ID;
        world::player_target::coord = voxel_pos();
        world::player_target::normal = voxel_pos();
        world::player_target::info = nullptr;
    }
}

void world::player_target::render(void)
{
    if((world::player_target::voxel != NULL_VOXEL_ID) && !client_game::hide_hud) {
        auto cpos = coord::to_chunk(world::player_target::coord);
        auto fpos = coord::to_local(world::player_target::coord);

        world::outline::prepare();
        world::outline::cube(cpos, glm::fvec3(fpos), glm::fvec3(1.0f), 2.0f, glm::fvec4(0.0f, 0.0f, 0.0f, 1.0f));
    }
}
