#include "client/pch.hh"

#include "client/game.hh"

#include "core/identifier.hh"
#include "core/utils/angles.hh"

#include "shared/utils/coord.hh"

#include "shared/block_registry.hh"
#include "shared/constant.hh"
#include "shared/coord.hh"
#include "shared/head.hh"
#include "shared/player.hh"
#include "shared/transform.hh"
#include "shared/velocity.hh"
#include "shared/world.hh"

#include "client/camera.hh"
#include "client/globals.hh"

constexpr static float CAM_SENSITIVITY = 0.125f;
constexpr static float CAM_SPEED = 6.0f;

constexpr static float MIN_PITCH = utils::radians(-90.0f);
constexpr static float MAX_PITCH = utils::radians(90.0f);

static void on_mouse_motion(const SDL_MouseMotionEvent& event)
{
    auto sensitivity = utils::radians(CAM_SENSITIVITY);

    camera::angles.x() = std::clamp(camera::angles.x() - sensitivity * event.yrel, MIN_PITCH, MAX_PITCH);
    camera::angles.y() -= sensitivity * event.xrel;
}

static void generate_debug_terrain(void)
{
    auto stone_id = block_registry::find(Identifier::from_string("builtin:stone"));
    auto slab_id = block_registry::find(Identifier::from_string("builtin:stone_slab"));

    if(stone_id == BLOCK_ID_NULL || slab_id == BLOCK_ID_NULL) {
        LOG_WARNING("builtin:stone/stone_slab not found, skipping debug terrain");
        return;
    }

    const chunk_pos cpos(2, 2, 2);
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
    globals::dispatcher.sink<SDL_MouseMotionEvent>().connect<&on_mouse_motion>();

    camera::init();
}

void client_game::init_late(void)
{
    generate_debug_terrain();

    // start the camera near the debug terrain patch at chunk_pos(2,2,2),
    // or there's nothing in view
    camera::chunk = chunk_pos(2, 2, 2);
    camera::local = Eigen::Vector3f(-8.0f, 24.0f, -8.0f);
    camera::angles = Eigen::Vector3f(utils::radians(-90.0f), utils::radians(45.0f), 0.0f);
}

void client_game::shutdown(void)
{
}

void client_game::update(void)
{
    Eigen::Vector3f wishdir(Eigen::Vector3f::Zero());

    const auto keys = SDL_GetKeyboardState(nullptr);

    if(keys[SDL_SCANCODE_W]) {
        wishdir += camera::forward;
    }

    if(keys[SDL_SCANCODE_S]) {
        wishdir -= camera::forward;
    }

    if(keys[SDL_SCANCODE_A]) {
        wishdir -= camera::right;
    }

    if(keys[SDL_SCANCODE_D]) {
        wishdir += camera::right;
    }

    if(keys[SDL_SCANCODE_SPACE]) {
        wishdir += camera::up;
    }

    if(wishdir.squaredNorm()) {
        camera::local += globals::window_frametime * CAM_SPEED * wishdir.normalized();
    }

    camera::update();
}

void client_game::update_late(void)
{
    SDL_SetWindowRelativeMouseMode(globals::window, true);
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
}
