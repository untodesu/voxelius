#include "client/pch.hh"

#include "client/game.hh"

#include "shared/identifier.hh"

#include "client/globals.hh"

void client_game::init(void)
{
    Identifier i;

    i = Identifier::from_string("voxelius:some_identifier");
    LOG_INFO("{}", i.full_string());

    i = Identifier::from_string(":invalid_identifier");
    LOG_INFO("{}", i.full_string());

    i = Identifier::from_string("something", "default_namespace");
    LOG_INFO("{}", i.full_string());

    i = Identifier::from_string("voxelius:something", "default_namespace");
    LOG_INFO("{}", i.full_string());

    i = Identifier::from_string("builtin:cube");
    LOG_INFO("{}", i.as_file_path("models/block", ".json"));
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
    // empty
}

void client_game::update_late(void)
{
    // empty
}

void client_game::fixed_update(void)
{
    // empty
}

void client_game::fixed_update_late(void)
{
    // empty
}

void client_game::layout(void)
{
    ImGui::Text("skibidi sigma %f", 1.0f / globals::window_frametime_avg);
}
