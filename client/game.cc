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
#include "client/language.hh"
#include "client/net/bother.hh"
#include "client/system/interpolation.hh"
#include "client/system/player_look.hh"
#include "client/system/player_move.hh"
#include "client/system/player_target.hh"

static void on_bother_response(const BotherResponseEvent& event)
{
    if(event.unreachable()) {
        LOG_WARNING("bother test: request {} unreachable", event.request_id());
        return;
    }

    LOG_INFO("bother test: request {} -> {}.{}.{}, {}/{} players, motd: \"{}\"", event.request_id(), event.version_major(),
        event.version_minor(), event.version_patch(), event.num_players(), event.max_players(), event.motd());
}

void client_game::init(void)
{
    interpolation::init();

    player_look::init();
    player_move::init();
    player_target::init();

    globals::dispatcher.sink<BotherResponseEvent>().connect<&on_bother_response>();

    bother::ping(1, "127.0.0.1", 16384);
}

void client_game::init_late(void)
{
    // empty
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
