#include "client/pch.hh"

#include "client/game.hh"

#include "core/identifier.hh"
#include "core/utils/angles.hh"

#include "shared/utils/coord.hh"

#include "shared/block_registry.hh"
#include "shared/coord.hh"
#include "shared/transform.hh"
#include "shared/velocity.hh"
#include "shared/world.hh"

#include "client/gui/gui.hh"

#include "client/utils/entity.hh"

#include "client/camera.hh"
#include "client/globals.hh"
#include "client/interpolation.hh"
#include "client/player_look.hh"
#include "client/player_move.hh"

static void generate_debug_terrain(void)
{
    auto stone_id = block_registry::find(Identifier::from_string("builtin:stone"));
    auto slab_id = block_registry::find(Identifier::from_string("builtin:stone_slab"));

    if(stone_id == BLOCK_ID_NULL || slab_id == BLOCK_ID_NULL) {
        LOG_WARNING("builtin:stone/stone_slab not found, skipping debug terrain");
        return;
    }

    const chunk_pos cpos(0, 0, 0);
    world::create_chunk(cpos);

    constexpr std::int32_t SIZE = 32;

    for(std::int32_t x = 0; x < SIZE; x += 1) {
        for(std::int32_t z = 0; z < SIZE; z += 1) {
            world::set_block(cpos, local_pos(x, 0, z), stone_id);

            const auto slab_bpos = utils::to_block(cpos, local_pos(x, 1, z));
            world::set_block(slab_bpos, slab_id);

            switch((x * SIZE + z) % 3) {
                case 0:
                    // "bottom" is the default, nothing to do
                    break;

                case 1:
                    world::set_state(slab_bpos, "orientation", "top");
                    break;

                default:
                    world::set_state(slab_bpos, "orientation", "double");
                    break;
            }
        }
    }
}

void client_game::init(void)
{
    player_look::init();
    player_move::init();
}

void client_game::init_late(void)
{
    globals::player = utils::spawn_player_client({ -8, 24, -8 });

    generate_debug_terrain();

    gui::screen = GUI_SCREEN_NONE;
}

void client_game::shutdown(void)
{
    // empty
}

void client_game::update(void)
{
    interpolation::update();
}

void client_game::update_late(void)
{
    player_look::update_late();
    player_move::update_late();
}

void client_game::fixed_update(void)
{
    player_move::fixed_update();

    Transform::fixed_update();
    Velocity::fixed_update();

    world::fixed_update();
}

void client_game::fixed_update_late(void)
{
    // empty
}

void client_game::layout(void)
{
    // empty
}
