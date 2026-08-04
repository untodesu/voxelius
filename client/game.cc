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

#include "client/camera.hh"
#include "client/globals.hh"
#include "client/gui/gui.hh"
#include "client/system/interpolation.hh"
#include "client/system/player_look.hh"
#include "client/system/player_move.hh"
#include "client/system/player_target.hh"

void client_game::init(void)
{
    interpolation::init();

    player_look::init();
    player_move::init();
    player_target::init();
}

void client_game::init_late(void)
{
    // TODO: spawn the local player entity through the entities/class system
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
