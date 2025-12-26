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

voxel_pos player_target::coord;
voxel_pos player_target::normal;
const Voxel* player_target::voxel;

void player_target::init(void)
{
    player_target::coord = voxel_pos();
    player_target::normal = voxel_pos();
    player_target::voxel = nullptr;
}

void player_target::update(void)
{
    if(session::is_ingame()) {
        RayDDA ray(globals::dimension, camera::position_chunk, camera::position_local, camera::direction);

        do {
            player_target::voxel = ray.step();

            if(player_target::voxel) {
                player_target::coord = ray.vpos;
                player_target::normal = ray.vnormal;
                break;
            }

            player_target::coord = voxel_pos();
            player_target::normal = voxel_pos();
        } while(ray.distance < MAX_REACH);
    }
    else {
        player_target::voxel = nullptr;
        player_target::coord = voxel_pos();
        player_target::normal = voxel_pos();
    }
}

void player_target::render(void)
{
    if(player_target::voxel && !client_game::hide_hud) {
        auto cpos = coord::to_chunk(player_target::coord);
        auto fpos = coord::to_local(player_target::coord);

        outline::prepare();
        outline::cube(cpos, glm::fvec3(fpos), glm::fvec3(1.0f), 2.0f, glm::fvec4(0.0f, 0.0f, 0.0f, 1.0f));
    }
}
