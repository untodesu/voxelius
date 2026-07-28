#include "client/pch.hh"

#include "client/game.hh"

#include "core/camera.hh"
#include "core/identifier.hh"
#include "core/utils/angles.hh"

#include "shared/constant.hh"
#include "shared/coord.hh"
#include "shared/physics/physics.hh"
#include "shared/utils/coord.hh"
#include "shared/utils/world.hh"
#include "shared/world/block_registry.hh"
#include "shared/world/climate.hh"
#include "shared/world/worldgen.hh"

#include "client/camera.hh"
#include "client/entity/interpolation.hh"
#include "client/entity/player_look.hh"
#include "client/entity/player_move.hh"
#include "client/entity/player_target.hh"
#include "client/globals.hh"
#include "client/gui/gui.hh"
#include "client/utils/entity.hh"

static void generate_debug_terrain(void)
{
    constexpr static ChunkPos::value_type CHUNK_RADIUS = 32;
    constexpr static ChunkPos::value_type VERT_RADIUS = 4;

    for(ChunkPos::value_type cx = -CHUNK_RADIUS; cx <= CHUNK_RADIUS; cx += 1) {
        for(ChunkPos::value_type cz = -CHUNK_RADIUS; cz <= CHUNK_RADIUS; cz += 1) {
            for(ChunkPos::value_type cy = -VERT_RADIUS; cy <= VERT_RADIUS; cy += 1) {
                worldgen::request({ cx, 20 + cy, cz });
            }
        }
    }
}

void client_game::init(void)
{
    player_look::init();
    player_move::init();
    player_target::init();
}

void client_game::init_late(void)
{
    globals::player = utils::spawn_player_client({ -8, 24, -8 });

    generate_debug_terrain();
}

void client_game::shutdown(void)
{
    // empty
}

void client_game::update(void)
{
    ZoneScoped;

    interpolation::update();

    player_target::update();
}

void client_game::update_late(void)
{
    ZoneScoped;

    player_look::update_late();
    player_move::update_late();
}

void client_game::fixed_update(void)
{
    ZoneScoped;

    player_move::fixed_update();
}

void client_game::fixed_update_late(void)
{
    ZoneScoped;

    // empty
}

void client_game::layout(void)
{
    // empty
}

void client_game::render(void)
{
    ZoneScoped;

    player_target::render();
}
