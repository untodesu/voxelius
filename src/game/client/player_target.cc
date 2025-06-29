#include "client/pch.hh"

#include "client/player_target.hh"

#include "shared/coord.hh"
#include "shared/dimension.hh"
#include "shared/ray_dda.hh"

#include "client/camera.hh"
#include "client/game.hh"
#include "client/globals.hh"
#include "client/outline.hh"
#include "client/session.hh"

constexpr static float MAX_REACH = 16.0f;

voxel_id player_target::voxel;
voxel_pos player_target::coord;
voxel_pos player_target::normal;
const VoxelInfo* player_target::info;

void player_target::init(void)
{
    player_target::voxel = NULL_VOXEL_ID;
    player_target::coord = voxel_pos();
    player_target::normal = voxel_pos();
    player_target::info = nullptr;
}

void player_target::update(void)
{
    if(session::is_ingame()) {
        RayDDA ray(globals::dimension, camera::position_chunk, camera::position_local, camera::direction);

        do {
            player_target::voxel = ray.step();

            if(player_target::voxel != NULL_VOXEL_ID) {
                player_target::coord = ray.vpos;
                player_target::normal = ray.vnormal;
                player_target::info = voxel_registry::find(player_target::voxel);
                break;
            }

            player_target::coord = voxel_pos();
            player_target::normal = voxel_pos();
            player_target::info = nullptr;
        } while(ray.distance < MAX_REACH);
    } else {
        player_target::voxel = NULL_VOXEL_ID;
        player_target::coord = voxel_pos();
        player_target::normal = voxel_pos();
        player_target::info = nullptr;
    }
}

void player_target::render(void)
{
    if((player_target::voxel != NULL_VOXEL_ID) && !client_game::hide_hud) {
        auto cpos = coord::to_chunk(player_target::coord);
        auto fpos = coord::to_local(player_target::coord);

        outline::prepare();
        outline::cube(cpos, glm::fvec3(fpos), glm::fvec3(1.0f), 2.0f, glm::fvec4(0.0f, 0.0f, 0.0f, 1.0f));
    }
}
