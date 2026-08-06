#include "server/pch.hh"

#include "server/game.hh"

#include "core/identifier.hh"

#include "shared/utils/entity.hh"

void server_game::init(void)
{
    // empty
}

void server_game::init_late(void)
{
    utils::spawn(Identifier::from_string("builtin:test_entity"));
}

void server_game::shutdown(void)
{
    // empty
}

void server_game::fixed_update(void)
{
    ZoneScoped;
}

void server_game::fixed_update_late(void)
{
    ZoneScoped;
}
