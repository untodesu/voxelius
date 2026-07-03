#include "client/pch.hh"

#include "client/game.hh"

#include "client/globals.hh"

void client_game::init(void)
{
    // empty
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
